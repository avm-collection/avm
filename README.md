<p align="center">
	<img width="350px" src="res/logo.png">
</p>
<p align="center">A stack based virtual machine</p>

<p align="center">
	<a href="./LICENSE">
		<img alt="License" src="https://img.shields.io/badge/license-GPL-blue?color=26d374"/>
	</a>
	<a href="https://github.com/avm-collection/avm/issues">
		<img alt="Issues" src="https://img.shields.io/github/issues/avm-collection/avm?color=4f79e4"/>
	</a>
	<a href="https://github.com/avm-collection/avm/pulls">
		<img alt="GitHub pull requests" src="https://img.shields.io/github/issues-pr/avm-collection/avm?color=4f79e4"/>
	</a>
</p>

A simple stack based virtual machine written in C.

## Table of contents
* [Write a program](#write-a-program)
* [Quickstart](#quickstart)
* [Milestones](#milestones)
* [Documentation](#documentation)
* [Bugs](#bugs)
* [Make](#make)

## Write a program
To write a program for AVM, use the AVM assembler [ANASM](https://github.com/avm-collection/anasm).
There is also a statically typed compiled language being written for it, but it is not released yet.

## Quickstart
```sh
$ make
$ make install
$ avm PROGRAM
```

## Milestones
- [X] Basic stack operators
- [X] Executing from files
- [X] Conditional instructions, jumps
- [X] RAM
- [X] File reading, IO
- [X] Loading shared libraries
- [ ] Windows support

## Documentation
Hosted [here](https://avm-collection.github.io/avm/documentation)

## Bugs
If you find any bugs, please create an issue and report them.

## Make
Run `make all` to see all the make rules.
