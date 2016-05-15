#! /bin/bash
rsync -avhz out/encoded_chunk osu@rnc04.asusa.net:/home/rnccdn/encoded-files04/chunk

rsync -avhz out/encoded_chunk1 osu@rnc02.asusa.net:/home/rnccdn/encoded-files02/chunk

rsync -avhz out/encoded_chunk11 osu@rnc03.asusa.net:/home/rnccdn/encoded-files03/chunk
