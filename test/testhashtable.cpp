#include "../hashtable.h"
#include <assert.h>
#include <iostream>

using namespace mmtraining;
using namespace std;

class Key {
public:

    Key(int v = 0): value(v) {}

    bool operator==(const Key &snd) {
	return snd.GetValue() == GetValue();
    }

    int GetValue() const{
	return value;
    }

    unsigned int HashValue() {
	return value % 10;
    }

private:
    int value;
};

int main() {
    Key k1(200), k2(300);
    int v1 = 2, v2 = 3, v3;

    KVHashTable<Key, int, 1000> table;
    assert(table.Set(k1, v1) == 1);
    assert(table.Get(k1, v3) == true);
    assert(v3 == v1);

    assert(table.Get(k2, v3) == false);
    assert(table.Set(k2, v2) == 1);
    assert(table.Get(k2, v3) == true);
    assert(v3 == v2);

    assert(table.Set(k1, v2) == 2);
    assert(table.Get(k1, v3) == true);
    assert(v3 == v2);

    assert(table.Delete(k2) == true);
    assert(table.Get(k2, v3) == false);
    assert(table.Delete(k1) == true);
    assert(table.Get(k1, v3) == false);

    cout << "Test success" << endl;
    return 0;
}
