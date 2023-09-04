CFLAGS=-Wall -Werror -O2 -pipe

all: abstain

clean:
	rm abstain

readme:
	mandoc -T markdown abstain.1 > README.md
