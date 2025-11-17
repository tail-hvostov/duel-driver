counter=1
failed=0
for test in tests/compiled/*.out
do
    name=$(basename --suffix=".out" $test)
    echo "Test suite $counter: $name"
    counter=$((counter + 1))
    sudo $test
    exit_code=$?
    failed=$((failed+exit_code))
done
counter=$((counter - 1))
echo "Failed $failed/$counter tests."