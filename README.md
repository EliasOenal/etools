### What is etools about?
The etools are a handy collection of algorithms written in C.
They intend to be inclusive (but not limited) to the smallest of embedded systems and aim to comply with standard C99 whenever possible.
Providing essential building blocks, code quality is a major concern.
All of this is released into the public domain, in order to aid the use in free and commercial applications.

#### efilter
So far this is just a simple infinite impulse response low-pass filter, implemented using integer arithmetic.
This code was initially written for the multimon-ng project, where it has been in use for many years.
It was designed to be particularly efficient on the first generation Raspberry Pi, relying only on addition,
substraction and bit shifts in 32bit. It can utilise the ability of many ARM Processors to perform "free" shifts,
but overall performs well on most systems.

#### ecbuff
The main design goal of ecbuff is being a lock-free high-throughput inter-thread circular/ring buffer.
In addition to that it allows exposing of element pointers, enabling zero-copy operations and
direct peripheral or DMA access. For the most part ecbuff is written in plain C99,
with the notable exception of using memory barriers if selected. Alternatively multi-threading
can on some architectures be enabled using the volatile keyword, though this leaves the scope of the C standard.
It comes with a suite of tests and has been used in several commercial products.
