$CC -o bin/encryptDataDemo src/encryptDataDemo.c -I../include -L../lib -lviicsec -lmisbehaviorReport -laerolinkPKI_dummy -lv2xCryptoPal -lv2xCrypto -lpthread -lrt -lcrypto
$CC -o bin/encryptSpduDemo src/encryptSpduDemo.c -I../include -L../lib -lviicsec -lmisbehaviorReport -laerolinkPKI_dummy -lv2xCryptoPal -lv2xCrypto -lpthread -lrt -lcrypto
$CC -o bin/signMessageDemoAsync src/signMessageDemoAsync.c -I../include -L../lib -lviicsec -lmisbehaviorReport -laerolinkPKI_dummy -lv2xCryptoPal -lv2xCrypto -lpthread -lrt -lcrypto
$CC -o bin/signMessageDemo src/signMessageDemo.c -I../include -L../lib -lviicsec -lmisbehaviorReport -laerolinkPKI_dummy -lv2xCryptoPal -lv2xCrypto -lpthread -lrt -lcrypto
$CC -o bin/symmetricKeyEncryptDataDemo src/symmetricKeyEncryptDataDemo.c -I../include -L../lib -lviicsec -lmisbehaviorReport -laerolinkPKI_dummy -lv2xCryptoPal -lv2xCrypto -lpthread -lrt -lcrypto
$CC -o bin/symmetricKeyEncryptSpduDemo src/symmetricKeyEncryptSpduDemo.c -I../include -L../lib -lviicsec -lmisbehaviorReport -laerolinkPKI_dummy -lv2xCryptoPal -lv2xCrypto -lpthread -lrt -lcrypto
$CC -o bin/unsignedMessageDemo src/unsignedMessageDemo.c -I../include -L../lib -lviicsec -lmisbehaviorReport -laerolinkPKI_dummy -lv2xCryptoPal -lv2xCrypto -lpthread -lrt -lcrypto
