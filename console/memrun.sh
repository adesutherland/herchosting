rm core.*
valgrind ./console  --docroot . --http-address 0.0.0.0 --http-port 10000
