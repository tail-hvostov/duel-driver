mkdir -p tests/compiled;
for test in tests/*.c
do
    filename=$(basename --suffix ".c" "$test")
    out_file="tests/compiled/$filename.out"
    gcc $test -o "$out_file"
done
