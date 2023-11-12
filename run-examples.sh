#!/bin/sh

# Build examples

./build-examples.sh

echo "-----------------------------------------------------------"
echo " Example 01"
echo "-----------------------------------------------------------"

# Example one
./example_json_one

echo "-----------------------------------------------------------"
echo " Example 02"
echo "-----------------------------------------------------------"

# Example two
./example_json_omit_cols
