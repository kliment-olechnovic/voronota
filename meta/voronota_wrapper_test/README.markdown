"voronota_calculate_contacts.h" a simple wrapper for calculating contacts with Voronota.
It depends on the Voronota source code that should be downloaded from
"https://github.com/kliment-olechnovic/voronota/releases/download/v1.22.3149/voronota_1.22.3149.tar.gz"

"voronota_wrapper_test.cpp" is an example of how "voronota_calculate_contacts.h" can be used.

Command line to compile the example:

    g++ -O3 -o ./voronota_wrapper_test ./voronota_wrapper_test.cpp

Command line to run the example:

    cat ./test_data/input_balls | ./voronota_wrapper_test > ./test_data/output_contacts

