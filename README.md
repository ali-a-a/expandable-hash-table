# Expandable Hash Table

## About
This repository implements an expandable probing hash table, which uses thread helping approach to expand the table when needed.

## Evaluation
This hash table contains two different modes: Multi Start Expansion and 
Single Start Expansion. In the first one, all threads compete to start a new expansion phase, causing extra contention. In the second mode, one thread is designated for starting the expansion; other threads only help in this process. The following plot depicts the difference between these two modes:

<img width="512" alt="Plots" src="https://private-user-images.githubusercontent.com/68470999/423173240-bc1a648b-6ca3-4e31-a114-7467934300d0.png?jwt=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3NDIwODMwNzgsIm5iZiI6MTc0MjA4Mjc3OCwicGF0aCI6Ii82ODQ3MDk5OS80MjMxNzMyNDAtYmMxYTY0OGItNmNhMy00ZTMxLWExMTQtNzQ2NzkzNDMwMGQwLnBuZz9YLUFtei1BbGdvcml0aG09QVdTNC1ITUFDLVNIQTI1NiZYLUFtei1DcmVkZW50aWFsPUFLSUFWQ09EWUxTQTUzUFFLNFpBJTJGMjAyNTAzMTUlMkZ1cy1lYXN0LTElMkZzMyUyRmF3czRfcmVxdWVzdCZYLUFtei1EYXRlPTIwMjUwMzE1VDIzNTI1OFomWC1BbXotRXhwaXJlcz0zMDAmWC1BbXotU2lnbmF0dXJlPWNiOWFkZGRkMDJmNTUyN2UyYTIxM2JjYWJlZGFjYTdlZDA0MTNkYThhYWQxNzQ0MTQ1MjUzMDJjZGNiMjBkNjkmWC1BbXotU2lnbmVkSGVhZGVycz1ob3N0In0.xuQHN-fkKxQOMVvJdTwJ-DSbRF5rR_o_1_UFCyr-tek">

## Credits
The benchmark, util, and make files are provided by Prof. Trevor Brown.