#include<iostream>
#include<bits/stdc++.h>

using namespace std;

struct Bucket {
    int l;
    int capacity;
    vector<int> keys;
    Bucket(int c){
        l = 0;
        capacity = c;
    }

    Bucket(int ll, int c){
        l = ll;
        capacity = c;
    }
    bool put_key(int key){
        // The bucket is full
        if(keys.size() == capacity) return false;
        keys.push_back(key);
        return true;
    }
    void clear(){
        keys.clear();
    }
};

struct HashTable {
    int g;
    int bucket_capacity;
    vector<int> table;
    vector<Bucket> buckets;
    HashTable(int bc){
        g = 0;
        bucket_capacity = bc;
        table.push_back(0);
        buckets.push_back(Bucket(bucket_capacity));
    }

    int hash(int key){
        // Take last g bits
        return key & ((1 << g)-1);
    }

    void insert(int key){
        while(true){
            int bidx = table[hash(key)];
            if(buckets[bidx].put_key(key)) break;
            // Otherwise, we grow the table
            grow(key);
        }
    }
    void grow(int key){
        int bidx = table[hash(key)];
        if(g == buckets[bidx].l){
            // need to grow the table
            int n = 1 << g;
            int new_n = 1 << (++g);
            table.resize(new_n);
            for(int i = n; i < new_n; i++){
                // table[i] points to where table[i-n]s to
                table[i] = table[i-n];
            }
        }
        // Split the bucket at key
        split(key);
    }
    void split(int key){
        int bidx = table[hash(key)];
        // Copy the keys inside this bucket because they will be reorganized
        Bucket copied = buckets[bidx];
        // Clear the content of this bucket
        buckets[bidx].clear();

        // Create a new bucket
        int l = copied.l;
        buckets.push_back(Bucket(l, bucket_capacity));

        // Point last half of them to the last element
        // First half of pointers will still point to buckets[bidx]
        int num_ptrs = 1 << (g-l);
        assert(num_ptrs >= 2);
        for(int i = num_ptrs / 2; i < num_ptrs; i++){
            int idx = (i << l) | (hash(key) & ((1 << l) - 1));
            // This is where it original points to
            assert(table[idx] == bidx);
            // Point to the new bucket
            table[idx] = buckets.size()-1;
        }

        // Increment their local l counter
        buckets[bidx].l++;
        buckets.back().l++;

        // Re-put the content of copied back to these two buckets
        for(int k: copied.keys){
            buckets[table[hash(k)]].put_key(k);
        }
    }
};

ostream &operator<<(ostream &os, const HashTable &table){
    os << "==============================" << "\n";
    os << "g: " << table.g << "\n";
    os << "table_size: " << table.table.size() << "\n";
    int n = table.table.size();
    for(int i = 0; i < n; i++){
        cout << "table[" << i << "]: " << table.table[i] << "\n";
    }
    int b_n = table.buckets.size();
    for(int i = 0; i < b_n; i++){
        os << "buckets[" << i << "]:";
        for(int j = 0; j < table.buckets[i].keys.size(); j++){
            os << " "<< table.buckets[i].keys[j];
        }
        os << "\n";
    }
    os << "==============================" << "\n";
    return os;
}

int main(){
    vector<int> insert_nums = {15, 3, 7, 14, 1, 9, 23, 11, 17};
    HashTable ht(2);
    for(int num: insert_nums){
        ht.insert(num);
        cout << ht;
    }
    return 0;
}
