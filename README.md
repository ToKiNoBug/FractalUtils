# FractalUtils
 
This repo implements some tools that a fractal program may use, including color attaching, binary file specification and implementation, general 2d array, and png image generation.

## Binfile Specification
The bin file is a uncompressed binary data with multiple data blocks. The file should start with a 32 bytes header. The first 16 bytes must be the following magic numbers : 
```
{65, 114, 109, 97, 103, 101, 100, 100, 111, 110, 6, 6, 6, 42, 30, 0}
```

And the following 16 bytes remain unused.

Each data block starts with a 8 byte integer as its tag(int64_t), and a 8 byte unsigned integer(uint64_t) as the length in bytes of the data segment. All data should be stored in little-endian.