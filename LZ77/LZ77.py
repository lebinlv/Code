# coding utf-8

import math
from bitarray import bitarray
# bitarray API 使用说明: https://pypi.python.org/pypi/bitarray/
# windows 平台下安装 bitarray: https://www.lfd.uci.edu/~gohlke/pythonlibs/#bitarray

'''
    input stream: The sequence of bytes to be compressed.
            byte: The basic data element in the input stream.
 coding position: The position of the byte in the input stream that is currently being coded (the beginning of the lookahead buffer).
lookahead buffer: The byte sequence from the coding position to the end of the input stream.
                  # 即待编码区
          window: A buffer of size W that indicates the number of bytes from the coding position backward. 
                  The window is empty at the beginning of the compression, then grows to size W as the input stream is processed.
                  Once it reaches size W, it then "slides" along with the coding position
                  # 即可变字典区
术语定义来自 MSDN: https://msdn.microsoft.com/en-us/library/ee916854.aspx
'''

class LZ77Compressor:
    __MAX_WINDOW_SIZE = 8192              # 定义 window 的最大值，单位为 byte，8192 bytes 即 8KB
    __MAX_LOOKAHEAD_BUFFER_SIZE = 128     # 定义 lookahead_buffer 的最大值，单位为 byte

    def __init__(self, window_size=4096, lookahead_buffer_size=32):
        self.window_size = min(window_size, self.__MAX_WINDOW_SIZE)  # 默认值 4096 bytes, 故需要 12 bits 来存储 “偏移量”
        self.lookahead_buffer_size = min(lookahead_buffer_size, self.__MAX_LOOKAHEAD_BUFFER_SIZE) 
        # 默认值 32 bytes, 故需要 5 bits 来存储 “匹配长度”
        
        '''
        a. “偏移量” 和 “匹配长度” 的大小以 byte 为单位，故 4096 需要 12 bits 来表示 （2^12 = 4096）
        b. 题外话，UTF-8 编码的文件中，一个英文字符大小为 1byte （注：windows记事本中的 UTF-8 实际为 UTF-8 with BOM）
        c. 受上一条所述现象的影响，LZ77类中的 compress函数 的控制台字符输出功能仅支持部分编码格式。 压缩和解压功能不受影响
        d. 默认情况下的输出格式：
           “短语标记” 大小为 18 bits: 标志位 “1” (1 bit) + 偏移量(12 bits) + 匹配长度(5 bits);
           “符号标记” 大小为  9 bits: 标志位 “0” (1 bit) + 未匹配符号(8 bits)
        '''
        self.__offset_n = math.ceil(math.log2(self.window_size)) # 计算存储 “偏移量” 所需的 二进制位数
        self.__match_n = math.ceil(math.log2(self.lookahead_buffer_size))  # 计算存储 “匹配长度” 所需的 二进制位数

    def __longest_match(self, coding_position):
        '''
        寻找 window 和 lookahead_buffer 中的匹配字段, 若匹配字段长度大于等于2，返回 (偏移量, 匹配字段的长度)，否则返回 None
        之所以要求匹配字段长度大于2，是因为 匹配长度为1时，短语标记 比 字符标记 浪费存储空间
        '''
        offset = 0        # 初始化偏移量
        match = 0         # 初始化匹配长度
        longest_match = 1 # 匹配长度的最小值
        window_start_idx = max(0, coding_position-self.window_size) # 得到可变字典区首字符的下标

        # 遍历可变字典区
        for k in range(window_start_idx, coding_position):
            i = k
            j = 0
            match = 0
            while j < self.lookahead_buffer_size-1 and coding_position+j < self.__input_file_length:
                if self.__input_file_data[i] != self.__input_file_data[coding_position+j]:
                    break
                match += 1
                i += 1
                j += 1
            if match > longest_match:
                offset = coding_position-k
                longest_match = match
        if longest_match > 1:
            return(offset, longest_match)
        else:
            return None

    def __int_to_nbin(self, int_num, nbit):
        '''
        将整型数据 "int_num" 转为 nbit位 的二进制数据，返回数据类型为 bitarray
        '''

        nbit_bin = bitarray(endian='big')    # 初始化 bitarray
        nbit_bin.frombytes((int_num).to_bytes(2, byteorder='big'))   # 将 int_num 转化为 2 bytes(16 bits) 长度的二进制
        del nbit_bin[0:16-nbit]              # 删掉多余的位
        return nbit_bin

    def __bitarray_to_int(self, bitarray_data, nbit):
        '''
        将长度为 nbit 的 bitarray_data(其数据类型为bitarray) 转为整数
        '''
        int_value = 0
        for i in range(0, nbit):
            if bitarray_data[i]: int_value += 2**(nbit-1-i)
        return int_value

    def compress(self, input_file_path, output_file_path=None, get_data=False, display=False):
        '''
        a. 输入文件路径应为绝对路径，压缩后的文件为二进制文件
        b. 未指定输出文件路径时，将会在源文件所在目录下创建 “源文件名.LZ77” 作为输出文件
        c. 若 get_data = True，此函数将返回压缩后的数据, 数据类型为 bitarray
        d. 若 debug = True，此函数会将压缩后的数据形象化的显示在终端中
        '''

        self.__input_file_data = None    # 初始化输入文件数据
        self.__input_file_length = 0     # 初始化输入文件数据长度

        # 尝试以二进制只读方式读取输入文件内容
        try:
            with open(input_file_path, 'rb') as input_file:
                self.__input_file_data = input_file.read()
                self.__input_file_length = len(self.__input_file_data)    # 计算输入文件大小是多少 byte
        except IOError:
            print('Can not open '+input_file_path+' ...')
            raise

        coding_position = 0
        output_buffer = bitarray(endian='big')

        while coding_position < self.__input_file_length:
            match = self.__longest_match(coding_position) # 获取匹配信息

            if match:
                (match_offset, match_length) = match  # 获取偏移量和匹配长度
                output_buffer.append(True)            # 写 "1" 标志位
                output_buffer += self.__int_to_nbin(match_offset, self.__offset_n) # 写偏移量
                output_buffer += self.__int_to_nbin(match_length, self.__match_n)  # 写匹配长度

                if display and self.__input_file_length < 513:
                    print('<1, %d, %d>' %(match_offset, match_length), end=' ')
                coding_position += match_length
            else:
                output_buffer.append(False)    # 写 "0" 标志位
                output_buffer += self.__int_to_nbin(self.__input_file_data[coding_position], 8) # 写 未匹配的字符

                if display and self.__input_file_length < 513:
                    print('<0, %c>' %self.__input_file_data[coding_position], end=' ')
                coding_position += 1
        
        output_file_length = math.ceil(len(output_buffer)/8)               # 计算输出文件的大小，单位: byte
        ratio = (1 - output_file_length / self.__input_file_length) * 100  # 计算压缩率
        
        # 若未指定输出路径，则生成 .LZ77 文件
        output_file_path = output_file_path if output_file_path else input_file_path + '.LZ77'

        try:
            with open(output_file_path, 'wb') as output_file:
                output_buffer.tofile(output_file)
                print('\nFrom %d bytes to  %d bytes, you get a compression ratio of %.2f%% !!' 
                      %(self.__input_file_length, output_file_length, ratio))
                print('Compressed Successfully!')
                if get_data: return output_buffer
                else: return None
        except IOError:
            print('\nFile write error! Return compressed data.')
            return output_buffer

    def decompress(self, input_file_path, output_file_path=None, get_data=False):
        '''
        a. 输入文件路径应为绝对路径
        b. 未指定输出文件路径时，将会在源文件所在目录下创建 “源文件名.LZ77dec” 作为输出文件
        c. 若 get_data = True，此函数将返回解压后数据
        '''

        compressed_data = bitarray(endian='big') # 初始化用于保存压缩文件数据的 compressed_data
        output_buffer = bytearray()              # 初始化输出缓存

        # 尝试读取输入文件
        try:
            with open(input_file_path, 'rb') as input_file:
                compressed_data.fromfile(input_file)
                data_length = len(compressed_data)
        except IOError:
            print('Can not open '+input_file_path+' ...')
            raise

        # 因为压缩函数将数据写入文件中时，若写入数据大小不是 8 bits 的倍数时，会在其末尾添0以满足 8bits 的整倍数关系
        # 故将循环条件设置为 while data_length > 7
        while data_length > 7:
            flag = compressed_data.pop(0) # 读取标志位
            
            # 如果标志位是1，说明是 “短语标记”
            if flag: 
                offset = self.__bitarray_to_int(compressed_data[0:self.__offset_n], self.__offset_n) # 获取偏移量
                match_length = self.__bitarray_to_int(
                    compressed_data[self.__offset_n:self.__offset_n+self.__match_n], self.__match_n) # 获取匹配长度
                
                del compressed_data[0: self.__offset_n+self.__match_n]  # “短语标记” 读取完毕，将其删掉以便于下个循环继续读取
                data_length -= (self.__offset_n+self.__match_n+1)       # “短语标记” 被删掉，因此修改数据长度

                # 将 “短语标记” 解压缩
                for i in range(match_length):
                    output_buffer.append(output_buffer[-offset])
            
            # 如果标志位是0，说明是 “字符标记”
            else:
                output_buffer += compressed_data[0:8].tobytes()        # 因为是 “字符标记”，直接读取并写入 output_buffer 即可
                del compressed_data[0:8]                               # “字符标记” 读取完毕，将其删掉以便于下个循环继续读取
                data_length -= 9                                       # “字符标记” 被删掉，因此修改数据长度
        
        output_file_length = len(output_buffer)    # 计算输出文件大小，单位: bytes

        # 确定输出路径
        output_file_path = output_file_path if output_file_path else input_file_path + '.LZ77dec'

        # 尝试将 output_buffer 写入输出文件
        try:
            with open(output_file_path, 'wb') as output_file:
                output_file.write(output_buffer)
                print ('Decompressed successfully! The length of output file is %d bytes.' %output_file_length)
                if get_data: return output_buffer
                else: return None
        except IOError:
            print ('File write error...')
            return None
            raise

if __name__ == '__main__':
    test_compressor = LZ77Compressor()
    choice = input("Press 1 for compress or 2 for decompress: ")
    if choice == "1":
        input_file_path = input("Please input file\'s path which you want to compresse:\n" )
        test_compressor.compress(input_file_path, display=True)
    elif choice == "2":
        input_file_path = input("Please input file\'s path which you want to decompresse:\n" )
        test_compressor.decompress(input_file_path)

# bug exist..
