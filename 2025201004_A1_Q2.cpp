#include <iostream>
#include <vector>
#include <string>
using namespace std;

struct Node
{
    int next[26]; // Strores index of the child node
    bool flag;

    Node()
    {
        for(int i=0;i<26;i++)
        {
            next[i]=-1; // -1 means no child exists
        }
        flag=false;
    }
};

vector <Node> trie(1); // Start with one node (root)

// Inserting a word into the trie

void insert(string &s)
{
    int node=0;
    for(char ch : s)
    {
        int index=ch-'a';
        if(trie[node].next[index]==-1)
        {
            trie[node].next[index]=(int)trie.size();
            trie.push_back(Node());
        }
        node=trie[node].next[index];
    }
    trie[node].flag=true; // marks the end of the word
}

// Spell check operation

int spellCheck(string &t)
{
    int node=0;
    for(char ch : t)
    {
        int index=ch-'a';
        if(trie[node].next[index]==-1)
        {
            return 0; // path breaks
        }
        node=trie[node].next[index];
    }
    if(trie[node].flag)
    {
        return 1; // returns 1 only if exact end of word is reached
    }
    else
    {
        return 0;
    }
}

// DFS traversal for autocompleting word from prefix

void dfs(int node, string &path, vector<string> &results)
{
    if(trie[node].flag)
    {
        results.push_back(path); // word found
    }
    for(int i=0;i<26;i++)
    {
        if(trie[node].next[i]!=-1)
        {
            path.push_back('a'+i); // add the current letter
            dfs(trie[node].next[i], path, results);
            path.pop_back(); //backtrack
        }
    }
}

// Autocomplete operation

vector<string> autocomplete(string &prefix)
{
    int node=0;
    for(char ch : prefix)
    {
        int index=ch-'a';
        if(trie[node].next[index]==-1)
        {
            return {}; // there are no words with the prefix
        }
        node=trie[node].next[index];
    }
    vector <string> results;
    string path=prefix;
    dfs(node, path, results);
    return results;
}

// Autocorrect helper function. DFS traversal with DP based search: keep track of 
// edit distance row and traverse the trie while pruning impossible branches

void dfsDP(int node, string &word, vector<int> &prevRow, string &path, vector <string> &results)
{
    int m=(int)word.size();

    // Compute the DP row for this node (comparing "path" to "word")
    vector <int> currentRow(m+1);

    // Cost of converting path to empty prefix of "word"
    currentRow[0]=prevRow[0]+1;

    for(int j=1;j<=m;j++)
    {
        int insertCost=currentRow[j-1] + 1; // insertion
        int deleteCost=prevRow[j]+1; // deletion
        int replaceCost=prevRow[j-1]+(word[j-1]==path.back() ? 0 : 1); // match or substitute
        currentRow[j]=min(insertCost, min(deleteCost, replaceCost));
    }

    // If this node ends a word and edit distance <= 3, add to results
    if(trie[node].flag && currentRow[m]<=3)
    {
        results.push_back(path);
    }

    // Prune: if every value in this row > 3, no need to go deeper
    int minValue=currentRow[0];
    for(int j = 1; j <= m; j++)
    {
        if(currentRow[j]<minValue)
        {
            minValue=currentRow[j];
        }
    }
    if(minValue>3)
    {
        return;
    }

    // Recurse into children
    for(int i=0;i<26;i++)
    {
        int child=trie[node].next[i];
        if (child!=-1)
        {
            path.push_back('a' + i);
            dfsDP(child, word, currentRow, path, results);
            path.pop_back(); // backtrack
        }
    }
}

// Autocorrect operation

vector <string> autocorrect(string &word)
{
    int m=(int)word.size();

    // Initial DP row for empty path vs word
    vector <int> initRow(m+1);
    for(int j=0;j<=m;j++)
    {
        initRow[j]=j;
    }

    vector <string> results;
    string path;

    // Explore children of root
    for(int i=0;i<26;i++)
    {
        int child=trie[0].next[i];
        if(child!=-1)
        {
            path.push_back('a'+i);
            dfsDP(child, word, initRow, path, results);
            path.pop_back();
        }
    }

    return results;
}

// main function

int main()
{
    int n, q;
    cin>>n>>q;
    vector <string> output;
    
    // Insert all dictionary words into trie
    for(int i=0;i<n;i++)
    {
        string w;
        cin>>w;
        insert(w);
    }

    // Process queries
    for(int i=0;i<q;i++)
    {
        int type;
        string t;
        cin>>type>>t;

        if(type==1)
        {
            // Spell check
            // cout<<spellCheck(t)<<endl;
            output.push_back(to_string(spellCheck(t)));

        }
        else if(type==2)
        {
            // Autocomplete
            vector <string> results=autocomplete(t);
            output.push_back(to_string(results.size()));
            for(string &w : results)
            {
                //cout << w << endl;
                output.push_back(w);
            }

        }
        else if(type==3)
        {
            // Autocorrect
            vector <string> results=autocorrect(t);
            //cout<<results.size()<<endl;
            output.push_back(to_string(results.size()));
            for(string &w : results)
            {
                //cout<<w<<endl;
                output.push_back(w);
            }
        }
    }
    for(string &s : output)
    {
        cout<<s<<endl;
    }
    return 0;
}