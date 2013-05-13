rm core.*
killall -9 s3270
./web3270  --docroot . --http-address 0.0.0.0 --http-port 20000
#./console  --docroot . --http-address 0.0.0.0 --http-port 20000
