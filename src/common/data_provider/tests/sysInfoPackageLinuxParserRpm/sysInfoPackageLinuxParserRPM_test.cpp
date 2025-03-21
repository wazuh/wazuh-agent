#include "sysInfoPackageLinuxParserRPM_test.hpp"
#include "packages/berkeleyRpmDbHelper.h"
#include <db.h>
#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>

using ::testing::_; // NOLINT(bugprone-reserved-identifier)
using ::testing::AnyNumber;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

class UtilsMock
{
public:
    MOCK_METHOD(std::string, exec, (const std::string&, const size_t));
};

static UtilsMock*& GetGsUtilsMock()
{
    static UtilsMock* instance = nullptr;
    return instance;
}

std::string UtilsWrapper::exec(const std::string& cmd, const size_t bufferSize)
{
    return GetGsUtilsMock()->exec(cmd, bufferSize);
}

class RpmLibMock
{
public:
    MOCK_METHOD(int, rpmReadConfigFiles, (const char* file, const char* target));
    MOCK_METHOD(void, rpmFreeRpmrc, ());
    MOCK_METHOD(rpmtd, rpmtdNew, ());
    MOCK_METHOD(rpmtd, rpmtdFree, (rpmtd td));
    MOCK_METHOD(rpmts, rpmtsCreate, ());
    MOCK_METHOD(int, rpmtsOpenDB, (rpmts ts, int dbmode));
    MOCK_METHOD(int, rpmtsCloseDB, (rpmts ts));
    MOCK_METHOD(rpmts, rpmtsFree, (rpmts ts));
    MOCK_METHOD(int, headerGet, (Header h, rpmTagVal tag, rpmtd td, headerGetFlags flags));
    MOCK_METHOD(const char*, rpmtdGetString, (rpmtd td));
    MOCK_METHOD(uint64_t, rpmtdGetNumber, (rpmtd td));
    MOCK_METHOD(int, rpmtsRun, (rpmts ts, rpmps okProbs, rpmprobFilterFlags ignoreSet));
    MOCK_METHOD(rpmdbMatchIterator,
                rpmtsInitIterator,
                (rpmts ts, rpmDbiTagVal rpmtag, const void* keypointer, size_t keylen));
    MOCK_METHOD(Header, rpmdbNextIterator, (rpmdbMatchIterator mi));
    MOCK_METHOD(rpmdbMatchIterator, rpmdbFreeIterator, (rpmdbMatchIterator mi));
};

static RpmLibMock*& GetRpmLibMock()
{
    static RpmLibMock* instance = nullptr;
    return instance;
}

int rpmReadConfigFiles(const char* file, const char* target)
{
    return GetRpmLibMock()->rpmReadConfigFiles(file, target);
}

void rpmFreeRpmrc()
{
    GetRpmLibMock()->rpmFreeRpmrc();
}

rpmtd rpmtdNew()
{
    return GetRpmLibMock()->rpmtdNew();
}

rpmtd rpmtdFree(rpmtd td)
{
    return GetRpmLibMock()->rpmtdFree(td);
}

rpmts rpmtsCreate()
{
    return GetRpmLibMock()->rpmtsCreate();
}

int rpmtsOpenDB(rpmts ts, int dbmode)
{
    return GetRpmLibMock()->rpmtsOpenDB(ts, dbmode);
}

int rpmtsCloseDB(rpmts ts)
{
    return GetRpmLibMock()->rpmtsCloseDB(ts);
}

rpmts rpmtsFree(rpmts ts)
{
    return GetRpmLibMock()->rpmtsFree(ts);
}

int headerGet(Header h, rpmTagVal tag, rpmtd td, headerGetFlags flags)
{
    return GetRpmLibMock()->headerGet(h, tag, td, flags);
}

const char* rpmtdGetString(rpmtd td)
{
    return GetRpmLibMock()->rpmtdGetString(td);
}

uint64_t rpmtdGetNumber(rpmtd td)
{
    return GetRpmLibMock()->rpmtdGetNumber(td);
}

int rpmtsRun(rpmts ts, rpmps okProbs, rpmprobFilterFlags ignoreSet)
{
    return GetRpmLibMock()->rpmtsRun(ts, okProbs, ignoreSet);
}

rpmdbMatchIterator rpmtsInitIterator(rpmts ts, rpmDbiTagVal rpmtag, const void* keypointer, size_t keylen)
{
    return GetRpmLibMock()->rpmtsInitIterator(ts, rpmtag, keypointer, keylen);
}

Header rpmdbNextIterator(rpmdbMatchIterator mi)
{
    return GetRpmLibMock()->rpmdbNextIterator(mi);
}

rpmdbMatchIterator rpmdbFreeIterator(rpmdbMatchIterator mi)
{
    return GetRpmLibMock()->rpmdbFreeIterator(mi);
}

class LibDBMock
{
public:
    MOCK_METHOD(int, db_create, (DB**, DB_ENV*, u_int32_t));
    MOCK_METHOD(char*, db_strerror, (int));
    MOCK_METHOD(int, set_lorder, (DB*, int));
    MOCK_METHOD(int, open, (DB*, DB_TXN*, const char*, const char*, DBTYPE, u_int32_t, int));
    MOCK_METHOD(int, cursor, (DB*, DB_TXN*, DBC**, u_int32_t));
    MOCK_METHOD(int, c_get, (DBC*, DBT*, DBT*, u_int32_t));
    MOCK_METHOD(int, c_close, (DBC * cursor));
    MOCK_METHOD(int, close, (DB*, u_int32_t));
};

static LibDBMock*& GetLibDBMock()
{
    static LibDBMock* instance = nullptr;
    return instance;
}

int db_create(DB** dbp, DB_ENV* dbenv, u_int32_t flags)
{
    return GetLibDBMock()->db_create(dbp, dbenv, flags);
}

char* db_strerror(int error)
{
    return GetLibDBMock()->db_strerror(error);
}

static int DbSetLorder(DB* dbp, int lorder)
{
    return GetLibDBMock()->set_lorder(dbp, lorder);
}

static int DbOpen(DB* db, DB_TXN* txnid, const char* file, const char* database, DBTYPE type, u_int32_t flags, int mode)
{
    return GetLibDBMock()->open(db, txnid, file, database, type, flags, mode);
}

static int DbCursor(DB* db, DB_TXN* txnid, DBC** cursorp, u_int32_t flags)
{
    return GetLibDBMock()->cursor(db, txnid, cursorp, flags);
}

static int DbCGet(DBC* cursor, DBT* key, DBT* data, u_int32_t flags)
{
    return GetLibDBMock()->c_get(cursor, key, data, flags);
}

static int DbCClose(DBC* cursor)
{
    return GetLibDBMock()->c_close(cursor);
}

static int DbClose(DB* db, u_int32_t flags)
{
    return GetLibDBMock()->close(db, flags);
}

class CallbackMock
{
public:
    MOCK_METHOD(void, callbackMock, (nlohmann::json&), ());
};

TEST_F(SysInfoPackagesLinuxParserRPMTest, rpmFromBerkleyDB)
{
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast, cppcoreguidelines-avoid-magic-numbers,
    // cppcoreguidelines-pro-bounds-pointer-arithmetic)
    CallbackMock wrapper;

    auto expectedPackage1 =
        R"({"architecture":"amd64","description":"The Open Source Security Platform","format":"rpm","groups":"test","install_time":"5","name":"Wazuh","size":321,"vendor":"The Wazuh Team","version":"123:4.4-1","location":"","priority":null,"source":null})"_json;

    auto utils_mock {std::make_unique<UtilsMock>()};
    auto libdb_mock {std::make_unique<LibDBMock>()};

    GetGsUtilsMock() = utils_mock.get();
    GetLibDBMock() = libdb_mock.get();

    DB db {};
    DBC cursor {};

    db.set_lorder = DbSetLorder;
    db.open = DbOpen;
    db.cursor = DbCursor;
    db.close = DbClose;
    cursor.c_get = DbCGet;
    cursor.c_close = DbCClose;

    EXPECT_CALL(*fsw, exists(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*fsw, is_regular_file(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*libdb_mock, db_create(_, _, _)).Times(1).WillOnce(DoAll(SetArgPointee<0>(&db), Return(0)));
    EXPECT_CALL(*libdb_mock, set_lorder(_, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*libdb_mock, open(_, _, _, _, _, _, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*libdb_mock, cursor(_, _, _, _)).Times(1).WillOnce(DoAll(SetArgPointee<2>(&cursor), Return(0)));

    // Emulate data stored in database
    const std::string name {"Wazuh"};
    const std::string version {"4.4"};
    const std::string release {"1"};
    const int epoch {123};
    const std::string summary {"The Open Source Security Platform"};
    const int itime {5};
    const int size {321};
    const std::string vendor {"The Wazuh Team"};
    const std::string group {"test"};
    const std::string source {"github"};
    const std::string arch {"amd64"};

    const auto total_fields {11};
    const auto total_fields_len {(name.length() + version.length() + release.length() + summary.length() +
                                  vendor.length() + group.length() + source.length() + arch.length() + 9) +
                                 (sizeof(epoch) + sizeof(itime) + sizeof(size))};

    const auto total_len {FIRST_ENTRY_OFFSET + ENTRY_SIZE * total_fields + total_fields_len + 1};

    DBT data {}, key {};
    std::vector<char> bytes(total_len, 0);
    size_t bytes_count {};

    char* cp = nullptr;
    int* ip = nullptr;

    data.data = bytes.data();
    data.size = static_cast<u_int32_t>(total_len);

    cp = bytes.data();

    auto entry {[&cp, &bytes_count](int tag, int type, size_t len)
                {
                    // Name
                    auto tmp = reinterpret_cast<int32_t*>(cp);
                    *tmp = static_cast<int32_t>(__builtin_bswap32(static_cast<uint32_t>(tag)));
                    cp += sizeof(int32_t);
                    // type
                    tmp = reinterpret_cast<int32_t*>(cp);
                    *tmp = static_cast<int32_t>(__builtin_bswap32(static_cast<uint32_t>(type)));
                    cp += sizeof(int32_t);
                    // offset
                    tmp = reinterpret_cast<int32_t*>(cp);
                    *tmp = static_cast<int32_t>(__builtin_bswap32(static_cast<uint32_t>(bytes_count)));
                    bytes_count += len;
                    cp += sizeof(int32_t);
                    // unused data
                    cp += sizeof(int32_t);
                }};

    auto content_string {[&cp](const std::string& value)
                         {
                             strcpy(cp, value.c_str());
                             cp += value.length() + 1;
                         }};
    auto content_int {[&cp](int value)
                      {
                          int32_t* tmp = reinterpret_cast<int32_t*>(cp);
                          *tmp = static_cast<int32_t>(__builtin_bswap32(static_cast<uint32_t>(value)));
                          cp += sizeof(int);
                      }};

    {
        // Header
        ip = reinterpret_cast<int32_t*>(cp);
        *ip = static_cast<int32_t>(__builtin_bswap32(static_cast<uint32_t>(total_fields)));
        cp += sizeof(int);
        ip = reinterpret_cast<int32_t*>(cp);
        *ip = static_cast<int32_t>(__builtin_bswap32(static_cast<uint32_t>(total_fields_len & 0xFFFFFFFF)));
        cp += sizeof(int);
    }

    entry(TAG_NAME, STRING_TYPE, name.length() + 1);
    entry(TAG_VERSION, STRING_TYPE, version.length() + 1);
    entry(TAG_RELEASE, STRING_TYPE, release.length() + 1);
    entry(TAG_EPOCH, INT32_TYPE, sizeof(epoch));
    entry(TAG_SUMMARY, STRING_TYPE, summary.length() + 1);
    entry(TAG_ITIME, INT32_TYPE, sizeof(itime));
    entry(TAG_SIZE, INT32_TYPE, sizeof(size));
    entry(TAG_VENDOR, STRING_TYPE, vendor.length() + 1);
    entry(TAG_GROUP, STRING_TYPE, group.length() + 1);
    entry(TAG_SOURCE, STRING_TYPE, source.length() + 1);
    entry(TAG_ARCH, STRING_TYPE, arch.length() + 1);

    content_string(name);
    content_string(version);
    content_string(release);
    content_int(epoch);
    content_string(summary);
    content_int(itime);
    content_int(size);
    content_string(vendor);
    content_string(group);
    content_string(source);
    content_string(arch);

    EXPECT_CALL(*libdb_mock, c_get(_, _, _, _))
        .Times(3)
        .WillOnce(DoAll(SetArgPointee<1>(key), SetArgPointee<2>(data), Return(0)))
        .WillOnce(DoAll(SetArgPointee<1>(key), SetArgPointee<2>(data), Return(0)))
        .WillOnce(Return(1));
    EXPECT_CALL(*libdb_mock, c_close(_)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*libdb_mock, close(_, _)).Times(1).WillOnce(Return(0));

    EXPECT_CALL(wrapper, callbackMock(expectedPackage1)).Times(1);

    GetRpmInfo([&wrapper](nlohmann::json& packageInfo) { wrapper.callbackMock(packageInfo); }, std::move(fsw));
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast, cppcoreguidelines-avoid-magic-numbers,
    // cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

TEST_F(SysInfoPackagesLinuxParserRPMTest, rpmFromLibRPM)
{
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast, cppcoreguidelines-avoid-magic-numbers)
    CallbackMock wrapper;

    auto expectedPackage1 =
        R"({"name":"1","architecture":"2","description":"3","size":4,"version":"5:7-6","vendor":"8","install_time":"9","groups":"10","format":"rpm","location":"","priority":null,"source":null})"_json;

    auto utils_mock {std::make_unique<UtilsMock>()};
    auto rpm_mock {std::make_unique<RpmLibMock>()};

    GetGsUtilsMock() = utils_mock.get();
    GetRpmLibMock() = rpm_mock.get();
    const auto ts = reinterpret_cast<rpmts>(0x123);
    const auto td = reinterpret_cast<rpmtd>(0x123);
    const auto mi = reinterpret_cast<rpmdbMatchIterator>(0x123);
    const auto header = reinterpret_cast<Header>(0x123);

    EXPECT_CALL(*fsw, exists(_)).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*rpm_mock, rpmReadConfigFiles(_, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*rpm_mock, rpmtsCreate()).Times(1).WillOnce(Return(ts));

    EXPECT_CALL(*rpm_mock, rpmtsOpenDB(_, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*rpm_mock, rpmtsRun(_, _, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*rpm_mock, rpmtdNew()).Times(1).WillOnce(Return(td));
    EXPECT_CALL(*rpm_mock, rpmtsInitIterator(_, _, _, _)).Times(1).WillOnce(Return(mi));
    EXPECT_CALL(*rpm_mock, rpmdbNextIterator(_)).WillOnce(Return(header)).WillOnce(Return(nullptr));
    EXPECT_CALL(*rpm_mock, rpmtsCloseDB(_)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*rpm_mock, rpmtsFree(_)).Times(1).WillOnce(Return(nullptr));
    EXPECT_CALL(*rpm_mock, rpmtdFree(_)).Times(1).WillOnce(Return(nullptr));
    EXPECT_CALL(*rpm_mock, rpmdbFreeIterator(_)).Times(1).WillOnce(Return(nullptr));
    EXPECT_CALL(*rpm_mock, rpmFreeRpmrc());

    EXPECT_CALL(*rpm_mock, headerGet(_, _, _, _)).Times(AnyNumber()).WillRepeatedly(Return(1));

    EXPECT_CALL(*rpm_mock, rpmtdGetString(_))
        .WillOnce(Return("1"))
        .WillOnce(Return("7"))
        .WillOnce(Return("6"))
        .WillOnce(Return("summary"))
        .WillOnce(Return("8"))
        .WillOnce(Return("10"))
        .WillOnce(Return("source"))
        .WillOnce(Return("2"))
        .WillOnce(Return("3"));
    EXPECT_CALL(*rpm_mock, rpmtdGetNumber(_)).WillOnce(Return(5)).WillOnce(Return(9)).WillOnce(Return(4));

    EXPECT_CALL(wrapper, callbackMock(expectedPackage1)).Times(1);

    GetRpmInfo([&wrapper](nlohmann::json& packageInfo) { wrapper.callbackMock(packageInfo); }, std::move(fsw));
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast, cppcoreguidelines-avoid-magic-numbers)
}

TEST_F(SysInfoPackagesLinuxParserRPMTest, rpmFallbackFromLibRPM)
{
    CallbackMock wrapper;

    auto expectedPackage1 =
        R"({"name":"1","architecture":"2","description":"3","size":4,"version":"5:7-6","vendor":"8","install_time":"9","groups":"10","format":"rpm","location":"","priority":null,"source":null})"_json;
    auto expectedPackage2 =
        R"({"name":"11","architecture":"12","description":"13","size":14,"version":"15:17-16","vendor":"18","install_time":"19","groups":"20","format":"rpm","location":"","priority":null,"source":null})"_json;

    auto utils_mock {std::make_unique<UtilsMock>()};
    auto rpm_mock {std::make_unique<RpmLibMock>()};

    GetGsUtilsMock() = utils_mock.get();
    GetRpmLibMock() = rpm_mock.get();

    EXPECT_CALL(*fsw, exists(_)).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*rpm_mock, rpmReadConfigFiles(_, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*utils_mock, exec(_, _))
        .Times(1)
        .WillOnce(Return("1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t\n11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t\n"));
    EXPECT_CALL(wrapper, callbackMock(expectedPackage1)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedPackage2)).Times(1);

    GetRpmInfo([&wrapper](nlohmann::json& packageInfo) { wrapper.callbackMock(packageInfo); }, std::move(fsw));
}

TEST_F(SysInfoPackagesLinuxParserRPMTest, rpmFallbackFromBerkleyDBConfigError)
{
    CallbackMock wrapper;

    auto expectedPackage1 =
        R"({"name":"1","architecture":"2","description":"3","size":4,"version":"5:7-6","vendor":"8","install_time":"9","groups":"10","format":"rpm","location":"","priority":null,"source":null})"_json;
    auto expectedPackage2 =
        R"({"name":"11","architecture":"12","description":"13","size":14,"version":"15:17-16","vendor":"18","install_time":"19","groups":"20","format":"rpm","location":"","priority":null,"source":null})"_json;

    auto utils_mock {std::make_unique<UtilsMock>()};
    auto libdb_mock {std::make_unique<LibDBMock>()};

    GetGsUtilsMock() = utils_mock.get();
    GetLibDBMock() = libdb_mock.get();

    EXPECT_CALL(*fsw, exists(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*fsw, is_regular_file(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*libdb_mock, db_create(_, _, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*libdb_mock, db_strerror(_))
        .Times(1)
        .WillOnce(Return(const_cast<char*>("test"))); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    EXPECT_CALL(*utils_mock, exec(_, _))
        .Times(1)
        .WillOnce(Return("1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t\n11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t\n"));
    EXPECT_CALL(wrapper, callbackMock(expectedPackage1)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedPackage2)).Times(1);

    GetRpmInfo([&wrapper](nlohmann::json& packageInfo) { wrapper.callbackMock(packageInfo); }, std::move(fsw));
}

TEST_F(SysInfoPackagesLinuxParserRPMTest, rpmFallbackFromBerkleyDBOpenError)
{
    CallbackMock wrapper;

    auto expectedPackage1 =
        R"({"name":"1","architecture":"2","description":"3","size":4,"version":"5:7-6","vendor":"8","install_time":"9","groups":"10","format":"rpm","location":"","priority":null,"source":null})"_json;
    auto expectedPackage2 =
        R"({"name":"11","architecture":"12","description":"13","size":14,"version":"15:17-16","vendor":"18","install_time":"19","groups":"20","format":"rpm","location":"","priority":null,"source":null})"_json;

    auto utils_mock {std::make_unique<UtilsMock>()};
    auto libdb_mock {std::make_unique<LibDBMock>()};

    GetGsUtilsMock() = utils_mock.get();
    GetLibDBMock() = libdb_mock.get();

    DB db {};

    db.set_lorder = DbSetLorder;
    db.open = DbOpen;
    db.close = DbClose;

    EXPECT_CALL(*fsw, exists(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*fsw, is_regular_file(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*libdb_mock, db_create(_, _, _)).Times(1).WillOnce(DoAll(SetArgPointee<0>(&db), Return(0)));
    EXPECT_CALL(*libdb_mock, set_lorder(_, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*libdb_mock, open(_, _, _, _, _, _, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*libdb_mock, db_strerror(_))
        .Times(1)
        .WillOnce(Return(const_cast<char*>("test"))); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    EXPECT_CALL(*libdb_mock, close(_, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*utils_mock, exec(_, _))
        .Times(1)
        .WillOnce(Return("1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t\n11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t\n"));
    EXPECT_CALL(wrapper, callbackMock(expectedPackage1)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedPackage2)).Times(1);

    GetRpmInfo([&wrapper](nlohmann::json& packageInfo) { wrapper.callbackMock(packageInfo); }, std::move(fsw));
}

TEST_F(SysInfoPackagesLinuxParserRPMTest, rpmFallbackFromBerkleyDBCursorError)
{
    CallbackMock wrapper;

    auto expectedPackage1 =
        R"({"name":"1","architecture":"2","description":"3","size":4,"version":"5:7-6","vendor":"8","install_time":"9","groups":"10","format":"rpm","location":"","priority":null,"source":null})"_json;
    auto expectedPackage2 =
        R"({"name":"11","architecture":"12","description":"13","size":14,"version":"15:17-16","vendor":"18","install_time":"19","groups":"20","format":"rpm","location":"","priority":null,"source":null})"_json;

    auto utils_mock {std::make_unique<UtilsMock>()};
    auto libdb_mock {std::make_unique<LibDBMock>()};

    GetGsUtilsMock() = utils_mock.get();
    GetLibDBMock() = libdb_mock.get();

    DB db {};

    db.set_lorder = DbSetLorder;
    db.open = DbOpen;
    db.close = DbClose;
    db.cursor = DbCursor;

    EXPECT_CALL(*fsw, exists(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*fsw, is_regular_file(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*libdb_mock, db_create(_, _, _)).Times(1).WillOnce(DoAll(SetArgPointee<0>(&db), Return(0)));
    EXPECT_CALL(*libdb_mock, set_lorder(_, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*libdb_mock, open(_, _, _, _, _, _, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*libdb_mock, cursor(_, _, _, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*libdb_mock, db_strerror(_))
        .Times(1)
        .WillOnce(Return(const_cast<char*>("test"))); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    EXPECT_CALL(*libdb_mock, close(_, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*utils_mock, exec(_, _))
        .Times(1)
        .WillOnce(Return("1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t\n11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t\n"));
    EXPECT_CALL(wrapper, callbackMock(expectedPackage1)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedPackage2)).Times(1);

    GetRpmInfo([&wrapper](nlohmann::json& packageInfo) { wrapper.callbackMock(packageInfo); }, std::move(fsw));
}

TEST_F(SysInfoPackagesLinuxParserRPMTest, emptyRpmFallback)
{
    CallbackMock wrapper;

    auto utils_mock {std::make_unique<UtilsMock>()};
    auto rpm_mock {std::make_unique<RpmLibMock>()};

    GetGsUtilsMock() = utils_mock.get();
    GetRpmLibMock() = rpm_mock.get();

    EXPECT_CALL(*fsw, exists(_)).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*rpm_mock, rpmReadConfigFiles(_, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*utils_mock, exec(_, _)).Times(1).WillOnce(Return(""));
    EXPECT_CALL(wrapper, callbackMock(_)).Times(0);

    GetRpmInfo([&wrapper](nlohmann::json& packageInfo) { wrapper.callbackMock(packageInfo); }, std::move(fsw));
}

TEST_F(SysInfoPackagesLinuxParserRPMTest, invalidPackageParsingRpmFallback)
{
    CallbackMock wrapper;

    auto utils_mock {std::make_unique<UtilsMock>()};
    auto rpm_mock {std::make_unique<RpmLibMock>()};
    auto libdb_mock {std::make_unique<LibDBMock>()};

    GetGsUtilsMock() = utils_mock.get();
    GetRpmLibMock() = rpm_mock.get();

    EXPECT_CALL(*fsw, exists(_)).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*rpm_mock, rpmReadConfigFiles(_, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*utils_mock, exec(_, _)).Times(1).WillOnce(Return("this is not a valid rpm -qa output"));
    EXPECT_CALL(wrapper, callbackMock(_)).Times(0);

    GetRpmInfo([&wrapper](nlohmann::json& packageInfo) { wrapper.callbackMock(packageInfo); }, std::move(fsw));
}
