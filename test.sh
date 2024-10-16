#!/Programming/bush

wget -q -O - https://en.wikipedia.org/wiki/Unix | grep -o -P '(?<=href=")http[^"]*' | cat -n