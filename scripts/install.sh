sed -i 's/\r$//' dependencies.txt
while IFS="" read -r p || [ -n "$p" ]
do
    arduino-cli lib install "$p"
done < dependencies.txt