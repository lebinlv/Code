# LZ77

## Dependencies

- Pyhton3
- bitarray

> Get bitarray module on Windows platform: https://www.lfd.uci.edu/~gohlke/pythonlibs/#bitarray

> Bitarray API: https://pypi.python.org/pypi/bitarray/

## Some Definition

- input stream: The sequence of bytes to be compressed.
- coding position: The position of the byte in the input stream that is currently being coded (the beginning of the lookahead buffer).
- lookahead buffer: The byte sequence from the coding position to the end of the input stream.
- window: A buffer of size W that indicates the number of bytes from the coding position backward. The window is empty at the beginning of the compression, then grows to size W as the input stream is processed.Once it reaches size W, it then "slides" along with the coding position

> 术语定义来自 MSDN: https://msdn.microsoft.com/en-us/library/ee916854.aspx

## Simple Test Example

![test1](https://s1.ax1x.com/2018/02/26/90s3Kx.png)

First type `python3 LZ77.py` in the shell, and then it will ask you as this:

    Press 1 for compress or 2 for decompress:
you should make a choice and here take 1 for example. Then input the file's path according to the promot infomation. Here the test file's path is `/home/lv/LZ77/test.txt`, which is in UTF-8 encoding. The file content is a string `ABCD?ABCDabcabcabcabcabcabc`. In test mode, it will print the compression description. In this example, it will print:

    <0, A> <0, B> <0, C> <0, D> <0, ?> <1, 5, 4> <0, a> <0, b> <0, c> <1, 3, 15>

## Usage

```python
  form LZ77 import LZ77Compressor
  compressor = LZ77Compressor(window_size=4096, lookahead_buffer_size=32)
```

- `window_size` and `lookahead_buffer_size` are optional, and their default value are 4096 and 32, which means that the window's size is 4096 bytes and lookahead-buffer's size is 32 bytes.

In defalut mode, the structure of a phrase token and a symbol token are:
![struction](https://s1.ax1x.com/2018/02/26/90ctaT.jpg)

### compress function

    compressor.compress(input_file_path[, output_file_path, get_data, display]):

- `output_file_path`, `get_data` and `display` are optional;
- when `output_file_path` is not given, it will creat `input_file_path.LZ77` as compressed file. Take `input_file_path='/home/lv/test.txt'` for example, the output file is '/home/lv/test.txt.LZ77';
- `get_data` is `False` in default, when enable `get_data`, the compress function will return a **bitarray**;
- `display` is `False` in default, when `display` is `True` and the size of input file is less than 513 bytes, the compression description is printed to standard output.

### decompress function

    compressor.decompress(input_file_path[, output_file_path, get_data])

- `output_file_path` and `get_data` are optional;
- when `output_file_path` is not given, it will creat `input_file_path.LZ77dec` as compressed file. Take `input_file_path='/home/lv/test.txt.LZ77'` for example, the output file is '/home/lv/test.txt.LZ77.dec';
- `get_data` is `False` in default, when enable `get_data`, the compress function will return a **bytearray**;

> **for more information, please read LZ77.py.**