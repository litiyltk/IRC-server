**установка drogon и других библиотек**
vcpkg install drogon boost-log boost-log-setup cpr jsoncpp ixwebsocket openssl


**сборка**
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build .

**регистрация**
curl -X POST http://localhost:8080/api/auth/register -H "Content-Type: application/json" -d "{\"login\":\"alice\", \"password\":\"1234\"}"

**вход**
curl -X POST http://localhost:8080/api/auth/login -H "Content-Type: application/json" -d "{\"login\":\"alice\", \"password\":\"1234\"}"

**получить список активных участников**
curl -X GET http://localhost:8080/api/users/online -H "Authorization: Bearer 131150aa402d23e42bf739cbdb1e51fe"

**отправка сообщения**
curl -X POST http://localhost:8080/api/messages -H "Authorization: Bearer 131150aa402d23e42bf739cbdb1e51fe" -H "Content-Type: application/json" -d "{\"text\":\"Привет, чат!\", \"to\":\"\"}"

**рассылка сообщения**

wscat -c "ws://localhost:8080/chat?token=131150aa402d23e42bf739cbdb1e51fe"

**выход**
curl -X POST http://localhost:8080/api/auth/logout -H "Authorization: Bearer 131150aa402d23e42bf739cbdb1e51fe"
curl -X POST http://localhost:8080/api/auth/logout -H "Authorization: Bearer e403df962d2ec7546b49d649c2aab776"