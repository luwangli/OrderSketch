## The OrderSketch

### Introduction

A **sketch** is a probabilistic data structure used to record frequencies of items in a multi-set. we propose a new sketch, **OrderSketch**, which has a simple structure and operation that is effortless to understand and use. And the OrderSketch is significantly faster than existing algorithms while maintaining high accuracy. Experimental results show that **OrderSketch** has about 3 times higher insertion speed compared with the state-of-the-art work.

This repo also contains a small demo to show how to use this algorithms with a  dataset.

### Requirements

- g++ >=5.4
- make >=4.1

### Hot to Build

We implement OrderSketch and 5 well known sketches (CM sketch, CU sketch, Count sketch, Pyramid sketch, ColdFilter) which are used for comparing with our proposed SF sketch. 
You can build thoses sketches by

```
$ cd demo;
$ make clean
$ make

```

### Usage

You can use the following commands to run

```
$cd demo
$ ./main.out
```

the result of folw frequency estimation are recorded clearly in various **txt** file, such as output_cm.txt record the estimate result with CM sketch.

