# Preface

The block allocator is represented as a shared library exposed through C functions. Inside, the memory is mmap'ed to a common file 'mempool', which enables multiprocess usage. The file consists of the blocks themselves followed by metadata that tracks used blocks. Note that both the data and metadata are modified in place by the interacting process(es). This means that: (1) the memory is insecure - it can be accessed by anyone and is affected by buffer overflows, (2) there is no extra entity that oversees and speedups memory operations (like tracking free blocks) thus performance is affected in scenarios with fragmentation and also one must always remember to free blocks when they are no longer needed (e.g. on exit). Regarding 8/16/32-bit architectures: it should work fine as long as the memory size defined in Makefile by block size and number doesn't exceed 2^CPU bitwidth.

# Configuring

In Makefile $block_size and $block_num variables can be changed to alter memory shape. In addition $verbose (default 1) can be set to 0 to omit internal library messages about block allocation and deallocation.

# Building

Use 'make all' to build all the components:

- Shared library.
- Test application (consists of 4 basic tests together).
- Memory file.

# Running

Add library path to env:

export LD_LIBRARY_PATH=`pwd`:$LD_LIBRARY_PATH

Run:

./test
