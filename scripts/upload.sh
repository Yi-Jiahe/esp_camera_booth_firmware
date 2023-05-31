sh ./scripts/compile.sh

PORT=$(arduino-cli board list | awk 'NR==2{print $1; exit}')

arduino-cli upload -p $PORT --profile $1 -v .