# Chain IO

## Introduction

This library allows buffer based input/output functions to be connected together. It provides a similar function to FILE pointers, but allows for direct buffer access. See chain-io/read.h to implement a process that reads from an abstracted source into a buffer, and chain-io/write.h to implement a process which writes from a buffer into an abstracted source. File descriptor reading and writing is implemented in the chain-io/fd/read.h and chain-io/fd/write.h headers.

\todo Provide a detailed workflow explanation