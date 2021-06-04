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

    void del(int key){
        auto it = find(keys.begin(), keys.end(), key);
        if(it != keys.end()){
            keys.erase(it);
        }
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

    bool find(int key){
        for(int k: buckets[table[hash(key)]].keys){
            if(k == key) return true;
        }
        return false;
    }

    void insert(int key){
        // This key already exists
        if(find(key)) return;

        while(true){
            int bidx = table[hash(key)];
            if(buckets[bidx].put_key(key)) break;
            // Otherwise, we grow the table
            grow(key);
        }
    }

    void del(int key, bool compress){
        // Not found, return
        if(!find(key)) return;
        buckets[table[hash(key)]].del(key);

        if(!compress) return;
        // Keep merging
        while(merge(key)){
        }
        // O(n): scan over the table
        int max_l = -1;
        for(int i: table){
            max_l = max(max_l, buckets[i].l);
        }
        // Resize the table and shrink to the maximum l size
        assert(max_l != -1);
        g = max_l;
        table.resize(1 << g);
    }

    bool merge(int key){
        // Search if we can compress the hash table
        // Ex.
        // Say: 00010, 01010, 10010, 11010 (l == 3) and g == 5
        //        ---    ---    ---    ---
        // And we want to consider a bucket that has the same l == 3 and share the same 2 bits
        // If that bucket exists, it should be like:
        //      ??110
        //        ---
        // Because its depth should be 3 and ??010 will all point to this bucket already
        // Futhermore, because ??110 is of depth 3
        //      00110, 01110, 10110, 11110
        //        ---    ---    ---    ---
        // Should all point to the same bucket as well
        int bidx = table[hash(key)];
        int l = buckets[bidx].l;
        if(l == 0) return false; // nothing to merge
        // Modify the l-1 bits. Ex: 010 -> 110
        int bidx2 = table[hash(key) ^ (1 << (l-1))];
        // Test if their depth are the same and one capacity is enough for two buckets' contents
        if(l == buckets[bidx2].l and buckets[bidx].keys.size() + buckets[bidx2].keys.size() <= bucket_capacity){
            // Merge them: put all contents into buckets[bidx]
            for(int k: buckets[bidx2].keys){
                assert(buckets[bidx].put_key(k));
            }
            // NOTE: this bucket will become somehow like a memory link
            //       In practice, I think vector<Bucket*> will be enough because we can free the content of Bucket
            //       Of course, there is still a Bucket* pointer left, but I think that will be enough.
            buckets[bidx2].l = -1; // Mark it as memory leak
            buckets[bidx2].clear();
            // Point them to bidx
            int num_ptrs = 1 << (g-l);
            for(int i = 0; i < num_ptrs; i++){
                int idx = (i << l) | ((hash(key) ^ (1 << (l-1))) & ((1 << l) - 1));
                assert(table[idx] == bidx2);
                table[idx] = bidx;
            }
            // Decrease it local counter
            buckets[bidx].l--;
            return true;
        }
        return false;
    }

    void grow(int key){
        int bidx = table[hash(key)];
        if(g == buckets[bidx].l){
            // need to grow the table
            int n = 1 << g;
            int new_n = 1 << (++g);
            table.resize(new_n);
            for(int i = n; i < new_n; i++){
                // table[i] points to where table[i-n] points to
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

        // Point odd half of them to the new bucket
        int num_ptrs = 1 << (g-l);
        assert(num_ptrs >= 2);
        // Ex: 0001, 0101, 1001, 1101 can point to the same bucket (if we consider 2 bits)
        //      ---   $$$   ---   $$$
        //     ^^^^        ^^^^       <----- Should point to different buckets after the split
        //           ****        **** <-----
        //      0     1     2     3
        //            ^           ^   <----- I let there points to the new bucket
        for(int i = 1; i < num_ptrs; i += 2){
            int idx = (i << l) | (hash(key) & ((1 << l) - 1));
            // This is where it original points to
            assert(table[idx] == bidx);
            // Point to the new bucket
            table[idx] = buckets.size()-1;
        }

        // Increment their local l counters
        buckets[bidx].l++;
        buckets.back().l++;

        // Re-put the keys back to these two buckets
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
        os << "buckets[" << i << "] (l == " << table.buckets[i].l << ") :";
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
    cout << "Question 3(a)-(b):" << "\n";
    for(int num: insert_nums){
        cout << "Insert: " << num << "\n";
        ht.insert(num);
        cout << ht;
    }
    cout << "Question 3(c):" << "\n";
    HashTable ht2(3);
    insert_nums = {2, 5, 13, 29, 7, 15, 1};
    for(int num: insert_nums){
        cout << "Insert: " << num << "\n";
        ht2.insert(num);
        cout << ht2;
    }
    vector<int> delete_nums = {2, 7, 13, 15};
    for(int dnum: delete_nums){
        ht2.del(dnum, true);
        cout << ht2;
    }
    return 0;
}
