# Expandable Hash Table

## About
This repository implements an expandable probing hash table, which uses thread helping approach to expand the table when needed.

## Evaluation
This hash table contains two different modes: Multi Start Expansion and 
Single Start Expansion. In the first one, all threads compete to start a new expansion phase, causing extra contention. In the second mode, one thread is designated for starting the expansion; other threads only help in this process. The following plot depicts the difference between these two modes:

<img width="512" alt="Plots" src="https://private-user-images.githubusercontent.com/68470999/423173240-bc1a648b-6ca3-4e31-a114-7467934300d0.png?jwt=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3NDMxMjM2OTksIm5iZiI6MTc0MzEyMzM5OSwicGF0aCI6Ii82ODQ3MDk5OS80MjMxNzMyNDAtYmMxYTY0OGItNmNhMy00ZTMxLWExMTQtNzQ2NzkzNDMwMGQwLnBuZz9YLUFtei1BbGdvcml0aG09QVdTNC1ITUFDLVNIQTI1NiZYLUFtei1DcmVkZW50aWFsPUFLSUFWQ09EWUxTQTUzUFFLNFpBJTJGMjAyNTAzMjglMkZ1cy1lYXN0LTElMkZzMyUyRmF3czRfcmVxdWVzdCZYLUFtei1EYXRlPTIwMjUwMzI4VDAwNTYzOVomWC1BbXotRXhwaXJlcz0zMDAmWC1BbXotU2lnbmF0dXJlPTk1M2YzMGU0OTM4NzIxZjExN2NkZWM3ZjEyNjAwNzhkNDc0ODBlMjU4YWYzZTMxNGQzYTk5NjdmOTM4NTNjNDkmWC1BbXotU2lnbmVkSGVhZGVycz1ob3N0In0.LWwc6RboqenmAu7tMsqSyYio1Jncb23Mfl9EJkfgEFg">

## Credits
The benchmark, util, and make files are provided by Prof. Trevor Brown.
