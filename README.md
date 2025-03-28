# Expandable Hash Table

## About
This repository implements an expandable probing hash table, which uses thread helping approach to expand the table when needed.

## Evaluation
This hash table contains two different modes: Multi Start Expansion and 
Single Start Expansion. In the first one, all threads compete to start a new expansion phase, causing extra contention. In the second mode, one thread is designated for starting the expansion; other threads only help in this process. The following plot depicts the difference between these two modes:

<img width="511" alt="423173240-bc1a648b-6ca3-4e31-a114-7467934300d0" src="https://github.com/user-attachments/assets/c4400f28-8d65-4faf-a8ac-c110d8c82d55" />

## Credits
The benchmark, util, and make files are provided by Prof. Trevor Brown.
