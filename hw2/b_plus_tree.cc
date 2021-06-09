#include <bits/stdc++.h>
using namespace std;

struct TreeNode {
    int n_child;
    int n_keys;
    bool is_leaf;
    TreeNode *next;
    vector<int> keys;
    vector<TreeNode*> children;
    vector<int> tuple_indices;

    TreeNode(bool il, int nc, int nk){
        n_child = nc;
        n_keys = nk;
        is_leaf = il;

        next = nullptr;
    }
    ~TreeNode(){
        for(TreeNode *c: children){
            delete c;
        }
    }

    bool is_full(){
        return keys.size() == n_keys;
    }

    bool criterion(){
        if(is_leaf) return keys.size() >= (n_keys + 1) / 2;
        return children.size() >= (n_child + 1) / 2;
    }
};

struct BPlusTree {
    int n_child;
    int n_keys;
    TreeNode *root;

    BPlusTree(int nc, int nk){
        assert(nc - 1 == nk);
        n_child = nc;
        n_keys = nk;
        root = nullptr;
    }
    ~BPlusTree(){
        if(root != nullptr) delete root;
    }

    TreeNode *_find_leaf(TreeNode *cur, int key, vector<TreeNode*> &path){
        if(cur == nullptr) return nullptr;
        path.push_back(cur);
        if(cur->is_leaf) return cur;
        // Linear search
        assert(cur->keys.size() > 0);
        // Ex. P K P K P (P for pointer, K for key)
        assert(cur->children.size() == cur->keys.size() + 1);
        int i;
        for(i = 0; i < cur->keys.size(); i++){
            if(key < cur->keys[i]) break;
        }
        return _find_leaf(cur->children[i], key, path);
    }

    void _put_key_into_leaf(TreeNode *L, int key, int tuple_idx){
        assert(L != nullptr and L->is_leaf);
        int i = 0;
        for(; i < L->keys.size(); i++){
            if(key < L->keys[i]) break;
        }
        L->keys.insert(L->keys.begin() + i, key);
        L->tuple_indices.insert(L->tuple_indices.begin() + i, tuple_idx);
    }

    void _put_key_into_nonleaf(
            TreeNode *parent, TreeNode *L, int new_key, TreeNode *new_L){
        assert(parent != nullptr and parent->children.size() > 0);
        // Linear search
        int i = 0;
        for(;i < parent->children.size(); i++){
            if(parent->children[i] == L) break;
        }
        assert(i <= parent->keys.size());
        parent->keys.insert(parent->keys.begin() + i, new_key);
        parent->children.insert(parent->children.begin() + i + 1, new_L);
    }

    void insert(int key, int tuple_idx){
        // If this tree is empty
        if(root == nullptr) root = new TreeNode(true, n_child, n_keys);
        vector<TreeNode*> path;
        TreeNode *L = _find_leaf(root, key, path);
        assert(L->is_leaf);
        if(!L->is_full()){
            _put_key_into_leaf(L, key, tuple_idx);
            assert(L->keys.size() == L->tuple_indices.size());
            return;
        }
        // Split this node into two
        _put_key_into_leaf(L, key, tuple_idx); // this TreeNode will be temporary overflow
        // ... -> L -> new_L -> ...
        TreeNode *new_L = new TreeNode(true, n_child, n_keys);
        // Copy half of key and tuple_indices to the right
        assert(L->keys.size() == n_keys+1);
        // Ex.
        // n_keys == odd  => (n_keys+1)/2, (n_keys+1)/2
        // n_keys == even => (n_keys+1+1)/2, (n_keys+1-1)/2
        int mid = (n_keys+1+1) / 2;
        for(int i = mid; i < n_keys+1; i++){
            new_L->keys.push_back(L->keys[i]);
            new_L->tuple_indices.push_back(L->tuple_indices[i]);
        }
        TreeNode *next = L->next;
        new_L->next = next;
        L->next = new_L;
        // Shrink L ( discard [(n_keys+1)/2, n_keys+1) )
        L->keys.resize(mid);
        L->tuple_indices.resize(mid);
        assert(new_L->keys.size() > 0);
        int new_key = new_L->keys[0]; // use this to split there two nodes
        insert_in_parent(L, new_key, new_L, path);
        return;
    }

    void insert_in_parent(TreeNode *L, int new_key, TreeNode *new_L, vector<TreeNode*> &path){
        assert(path.size() > 0 and path.back() == L);
        path.pop_back();
        TreeNode *parent = nullptr;
        if(path.size() > 0) parent = path.back();
        if(parent == nullptr){
            assert(L == root);
            TreeNode *new_root = new TreeNode(false, n_child, n_keys);
            parent = root = new_root;

            // L | new_key | new_L
            parent->keys.push_back(new_key);
            parent->children.push_back(L);
            parent->children.push_back(new_L);
            return;
        }

        assert(parent != nullptr);
        if(!parent->is_full()){
            // Insert (new_key, new_L) into the parent
            _put_key_into_nonleaf(parent, L, new_key, new_L);
            return;
        }
        // This will be temporary overflow
        _put_key_into_nonleaf(parent, L, new_key, new_L);
        assert(parent->keys.size() == n_keys+1);
        assert(parent->children.size() == n_child+1);
        TreeNode *new_parent = new TreeNode(false, n_child, n_keys);

        // Ex.
        // * indicates one is added
        // consider n_keys:
        //
        // even: 1 2 3 4 5*
        //           ^
        //      3 pointers on the left, 3 pointers on the right
        // odd: 1 2 3 4*
        //          ^
        //      3 pointers on the left, 2 pointers on the right
        int mid = (n_keys+1)/2;
        int mid_key = parent->keys[mid];

        // Copy half of the content to the right
        for(int i = mid+1; i < n_keys+1; i++){
            new_parent->keys.push_back(parent->keys[i]);
            new_parent->children.push_back(parent->children[i]);
        }
        new_parent->children.push_back(parent->children[n_keys+1]);

        parent->keys.resize(mid); // [i, mid)
        parent->children.resize(mid+1); // [i, mid+1)

        // Recursively update its parent
        insert_in_parent(parent, mid_key, new_parent, path);
    }
};

ostream &operator<<(ostream &os, const TreeNode *node){
    if(node == nullptr) return os;
    os << "Node Address: " << (void const *) node << "\n";
    cout << "Node Content:\n";
    if(!node->is_leaf){
        for(int i = 0; i < node->keys.size(); i++){
            os << (void const *) node->children[i] << "(addr) | ";
            os << node->keys[i] << "(key) | ";
        }
        os << (void const *) node->children[node->keys.size()] << "(addr)\n";
    }else{
        for(int i = 0; i < node->keys.size(); i++){
            os << node->tuple_indices[i] << "(tuple_idx) | ";
            os << node->keys[i] << "(key) | ";
        }
        os << "\n";
        os << "Next: " << (void const *) node->next << "\n";
    }
    return os;
}

ostream &operator<<(ostream &os, const BPlusTree &t){
    // BFS printing
    queue<TreeNode*> q;
    if(t.root != nullptr) q.push(t.root);
    int depth = 0;
    while(!q.empty()){
        os << "===================================\n";
        os << "Depth " << depth++ << ":\n";
        for(int i = q.size()-1; i >= 0; i--){
            TreeNode *node = q.front();
            q.pop();
            os << node << "\n";
            if(node->is_leaf) continue;
            for(TreeNode *child: node->children) q.push(child);
        }
        os << "===================================\n";
    }
    return os;
}

// Tuple simulates a database tuple (or record)
struct Tuple {
    int id;
    string content;
};

string random_str(size_t len){
    const char candidates[] = "abcdefghijklmnopqrstuvwxyz"; // there is a '\0' at the end
    const size_t n = sizeof(candidates) - 1;
    static default_random_engine r(0);
    static uniform_int_distribution<int> dist(0, n-1);
    auto generator = [candidates]() -> char {
        return candidates[dist(r)];
    };
    string s(len, 0);
    generate_n(s.begin(), len, generator);
    return s;
}

int main(){
    vector<Tuple> data;
    /* cout << "data:\n"; */
    /* for(int i = 0; i < 1000; i++){ */
    /*     data.push_back({i, random_str(5)}); */
    /*     cout << i << ": " << data[i].content << "\n"; */
    /* } */

    BPlusTree t(4, 3);
    vector<pair<int, int>> v = {{2, 2}, {3, 3}, {5, 5}, {7, 7}, {23, 23}, {19, 19}, {13, 13}, {17, 17},
                                {11, 11}, {12, 12}};
    for(auto &p: v){
        auto [k, tidx] = p;
        t.insert(k, tidx);
        cout << "Insert: " << "(" << k << "," << tidx << ")\n";
        cout << t;
    }

    BPlusTree t2(3, 2);
    v = {{3, 3}, {4, 4}, {5, 5}, {2, 2}, {1, 1}};
    for(auto &p: v){
        auto [k, tidx] = p;
        t2.insert(k, tidx);
        cout << "Insert: " << "(" << k << "," << tidx << ")\n";
        cout << t2;
    }
    return 0;
}
