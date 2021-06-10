#include <bits/stdc++.h>
using namespace std;

bool DEBUG = 1;

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

// A B+Tree that support insertion and deletion
// NOTE: This B+Tree does not support duplicate keys
struct BPlusTree {
    int n_child;
    int n_keys;
    TreeNode *root;

    BPlusTree(int nc, int nk){
        assert(nc - 1 == nk);
        assert(nk >= 1);
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

    bool del_key_from_leaf(TreeNode *L, int key, int tuple_idx){
        int i = 0;
        assert(L->keys.size() == L->tuple_indices.size());
        for(; i < L->keys.size(); i++){
            if(L->keys[i] == key and L->tuple_indices[i] == tuple_idx) break;
        }
        // Not found
        if(i == L->keys.size()) return false;

        // Delete (key, tuple_idx)
        L->keys.erase(L->keys.begin() + i);
        L->tuple_indices.erase(L->tuple_indices.begin() + i);
        return true;
    }

    // Tell this nonleaf node to delete (k, del_node)
    void del_in_nonleaf(TreeNode *node, int k, TreeNode *del_node, vector<TreeNode*> &path){
        assert(node != nullptr and !node->is_leaf);
        assert(path.back() == node);
        path.pop_back();
        // delete k
        int i = find(node->keys.begin(), node->keys.end(), k) - node->keys.begin();

        // k | del_node
        node->keys.erase(node->keys.begin() + i);
        assert(node->children[i+1] == del_node);
        node->children.erase(node->children.begin() + i + 1);

        // if this root has only one child, let this child be a root
        if(node == root){
            if(root->children.size() == 1){
                TreeNode *new_root = root->children[0];
                root->children.clear();
                delete root;
                root = new_root;
            }
            // otherwise, do nothing
            return;
        }

        // if this node has enough pointers (children)
        if(node->criterion()) return;

        assert(path.size() > 0);
        TreeNode *parent = path.back();
        TreeNode *sibling;
        int split_key;
        bool is_right;

        tie(sibling, split_key, is_right) = get_sibling(parent, node);
        assert(sibling != nullptr);

        if(sibling->children.size() + node->children.size() <= n_child){
            // Coalesce
            if(DEBUG) cout << "[DEBUG] Coalesce this nonleaf node with its sibling\n;"
            // Maintain this order: sibling | split_key | node
            if(is_right) swap(sibling, node);
            // Copy keys and children to the left sibling
            assert(node->keys.size() + 1 == node->children.size());

            // K'
            sibling->keys.push_back(split_key);
            for(int i = 0; i < node->keys.size(); i++){
                sibling->keys.push_back(node->keys[i]);
                sibling->children.push_back(node->children[i]);
            }
            sibling->children.push_back(node->children[node->keys.size()]);

            assert(sibling->keys.size() + 1 == sibling->children.size());

            // Tell its parent that split_key | node needs to be deleted
            del_in_nonleaf(parent, split_key, node, path);

            // clear its pointers (avoid destructor deletion)
            node->children.clear();
            delete node;
        }else{
            // Redistribute
            assert(sibling->children.size() > 0);
            if(is_right){
                if(DEBUG) cout << "[DEBUG] Redistribute this nonleaf node and is_right\n";
                // node | split_key | sibling
                node->keys.push_back(split_key);
                node->children.push_back(sibling->children[0]);

                int new_k = sibling->keys[0];

                // Left shift the sibling
                sibling->keys.erase(sibling->keys.begin());
                sibling->children.erase(sibling->children.begin());

                // Update the parent's split_key to new_k
                replace_parent_k(parent, split_key, new_k);
            }else{
                if(DEBUG) cout << "[DEBUG] Redistribute this nonleaf node and not is_right\n";
                // sibling | split_key | node
                node->keys.insert(node->keys.begin(), split_key);
                node->children.insert(node->children.begin(), sibling->children.back());

                int new_k = sibling->keys.back();

                // Remove the last element of sibling
                sibling->keys.pop_back();
                sibling->children.pop_back();

                // Update the parent's split_key to new_k
                replace_parent_k(parent, split_key, new_k);
            }
        }
    }

    // Get the left sibling first, if a left sibling doesn't exist, return the right one
    // the bool represent whether it is a right one
    tuple<TreeNode*, int, bool> get_sibling(
            TreeNode *parent, TreeNode *child){
        // There are at least two children (pointers)
        assert(parent->children.size() >= 2);

        auto it = find(parent->children.begin(), parent->children.end(), child);
        assert(it != parent->children.end());
        int i = it - parent->children.begin();
        // Choose the left one first
        if(i > 0){
            // sibling | key | child
            //                   ^
            //                   i
            return tuple<TreeNode*, int, bool>(
                    parent->children[i-1], parent->keys[i-1], false);
        }
        // Otherwise choose the right one
        // child | key | sibling
        //   ^
        //   i
        return tuple<TreeNode*, int, bool>(parent->children[i+1], parent->keys[i], true);
    }

    // replace parent's key k into new_k
    void replace_parent_k(TreeNode *parent, int k, int new_k){
        assert(parent != nullptr);
        auto it = find(parent->keys.begin(), parent->keys.end(), k);
        assert(it != parent->keys.end());
        int i = it - parent->keys.begin();
        parent->keys[i] = new_k;
    }

    void del(int key, int tuple_idx){
        if(root == nullptr) return;
        vector<TreeNode*> path;
        TreeNode *L = _find_leaf(root, key,  path);
        assert(L != nullptr);
        if(!del_key_from_leaf(L, key, tuple_idx)) return;
        // if this leaf is still half full
        if(L->criterion()) return;
        if(root == L){
            assert(root->is_leaf);
            if(root->keys.size() == 0){
                // Make this root a nullptr
                delete root;
                root = nullptr;
            }
            return;
        }
        // L has too few keys
        path.pop_back();
        assert(path.size() > 0);
        TreeNode *parent = path.back();

        TreeNode *sibling;
        int k;
        bool is_right;
        tie(sibling, k, is_right) = get_sibling(parent, L);
        assert(sibling != nullptr);

        // Case 1: the sibling has >  (n_keys+1)/2 => coalescing
        // Case 2: the sibling has == (n_keys+1)/2 => redistributing
        // NOTE: there is no Case 3!
        if(sibling->keys.size() + L->keys.size() <= n_keys){
            // Coalesce them
            // NOTE: Swap so that sibling will always be on the left (L will always be the one be deleted)
            // i.e. sibling | k | L
            //                    ^<--- delete this one
            if(DEBUG) cout << "[DEBUG] Coalesce this leaf node with its sibling\n";
            if(is_right) swap(sibling, L);
            for(int i = 0; i < L->keys.size(); i++){
                sibling->keys.push_back(L->keys[i]);
                sibling->tuple_indices.push_back(L->tuple_indices[i]);
            }
            TreeNode *next = L->next;
            sibling->next = next;

            // Tell its parent that (k, L) is deleted
            del_in_nonleaf(parent, k, L, path);
            // Then we can safely delete L
            delete L;
        }else{
            // Redistribute one element from the sibling
            assert(sibling->keys.size() >= 2);
            if(is_right){
                // Ex.
                // delete 2:
                //    |1 2 |  |4 5 6|
                //    |1 4 |  |5 6  |
                if(DEBUG) cout << "[DEBUG] Redistribute this leaf node and is_right\n";
                // L | k | sibling
                // steal the smallest element from its sibling
                L->keys.push_back(sibling->keys[0]);
                L->tuple_indices.push_back(sibling->tuple_indices[0]);

                // Left shift the sibling's keys and tuple_indices
                sibling->keys.erase(sibling->keys.begin());
                sibling->tuple_indices.erase(sibling->tuple_indices.begin());

                // The new splitting key
                int new_k = sibling->keys[0];

                // update parent's k to the new_k
                replace_parent_k(parent, k, new_k);
            }else{
                if(DEBUG) cout << "[DEBUG] Redistribute this leaf node and not is_right\n";
                // sibling | k | L
                // steal the largest element in the sibling
                L->keys.insert(L->keys.begin(), sibling->keys.back());
                L->tuple_indices.insert(L->tuple_indices.begin(), sibling->tuple_indices.back());

                // remove that element in the sibling
                sibling->keys.pop_back();
                sibling->tuple_indices.pop_back();

                int new_k = L->keys[0];

                // update parent's k to the new_k
                replace_parent_k(parent, k, new_k);
            }
        }
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

ostream &operator<<(ostream &os, const BPlusTree *t){
    return os << *t;
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

BPlusTree* generate_b_plus_tree(vector<pair<int, int>> &v){
    BPlusTree *t = new BPlusTree(4, 3);
    for(auto &p: v){
        t->insert(p.first, p.second);
    }
    return t;
}

void test_delete(){
    vector<pair<int, int>> v = {{5, 5}, {7, 7}, {10, 10}, {14, 14}, {17, 17}, {20, 20}, {16, 16}};

    cout << "Test redistribute leaf:\n";
    BPlusTree *t = generate_b_plus_tree(v);
    cout << t;
    t->del(7, 7);
    cout << t;
    delete t;

    v = {{5, 5}, {7, 7}, {10, 10}, {14, 14}, {17, 17}, {20, 20}, {11, 11}};
    cout << "Test redistribute leaf-2:\n";
    t = generate_b_plus_tree(v);
    cout << t;
    t->del(20, 20);
    cout << t;
    t->del(20, 20);
    cout << t;
    delete t;


    v = {{1, 1}, {5, 5}, {8, 8}, {12, 12}, {15, 15}, {19, 19}, {23, 23}, {26, 26},
         {30, 30}, {35, 35}, {20, 20}, {22, 22}, {21, 21}};
    cout << "Simulate textbook Figure 11.13:\n";
    t = generate_b_plus_tree(v);
    cout << t;

    cout << "Figure 11.16:\n";
    t->del(30, 30);
    cout << t;

    cout << "Figure 11.17:\n";
    t->del(26, 26);
    t->del(35, 35);
    cout << t;

    cout << "Figure 11.18:\n";
    t->del(20, 20);
    cout << t;

    delete t;
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

    test_delete();
    return 0;
}
