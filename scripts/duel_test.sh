counter=1
for test in tests/compiled/*.out
do
    name=$(basename --suffix=".out" $test)
    echo "Test $counter: $name"
    counter=$((counter + 1))
    sudo $test
done
