#include <iostream>
using namespace std;

// ----------------Global variables----------------

const int MAX_Q=100000;        // maximum queries
const int MAX_COORDS=2*MAX_Q; // maximum unique coordinates after compression
const int MAX_TREE_SIZE=8*MAX_COORDS; // safe size for segment tree

int queryType[MAX_Q + 5];
long long queryL[MAX_Q + 5];
long long queryR[MAX_Q + 5];
long long queryH[MAX_Q + 5];

// For coordinate compression
long long coordList[2*MAX_Q+5];  // store all unique coordinates
int coordCount=0;

// Segment Tree storage
long long tree[MAX_TREE_SIZE];
long long lazy[MAX_TREE_SIZE];

// -----------------Utility functions------------------

// Function to swap variables
void swapValues(long long &a, long long &b)
{
    long long temp=a;
    a=b;
    b=temp;
}

// Simple quicksort implementation for sorting coordList
int partitionArray(long long arr[], int low, int high)
{
    long long pivot=arr[high];
    int i=low-1;

    for(int j=low;j<high;j++)
    {
        if(arr[j]<=pivot)
        {
            i++;
            swapValues(arr[i], arr[j]);
        }
    }
    swapValues(arr[i+1], arr[high]);
    return i+1;
}

void quickSort(long long arr[], int low, int high)
{
    if(low<high)
    {
        int pi=partitionArray(arr, low, high);
        quickSort(arr, low, pi-1);
        quickSort(arr, pi+1, high);
    }
}

// Remove duplicates from sorted coordList
int removeDuplicates(long long arr[], int n)
{
    if(n==0)
    {
        return 0;
    }
    int newSize=1;
    for(int i=1;i<n;i++)
    {
        if(arr[i]!=arr[newSize-1])
        {
            arr[newSize]=arr[i];
            newSize++;
        }
    }
    return newSize;
}

// Find index of a coordinate in compressed array (binary search)
int findIndex(long long arr[], int n, long long value)
{
    int low=0;
    int high=n-1;
    while(low<=high)
    {
        int mid=(low+high)/2;
        if(arr[mid]==value)
        {
            return mid;
        }
        else if(arr[mid]<value)
        {
            low=mid+1;
        }
        else
        {
            high=mid-1;
        }
    }
    return -1; // should never happen if compression done correctly
}

// ---------------Segment Tree functions-------------------

// Push lazy value down to children
void pushDown(int node, int leftChild, int rightChild)
{
    if(lazy[node]!=-1)
    {
        // Assign lazy value to children
        tree[leftChild]=lazy[node];
        tree[rightChild]=lazy[node];
        lazy[leftChild]=lazy[node];
        lazy[rightChild]=lazy[node];

        // Clear lazy value of current node
        lazy[node]=-1;
    }
}

// Range update: set height for interval [l, r]
void updateRange(int node, int start, int end, int l, int r, long long height)
{
    if(r<start || end<l)
    {
        // No overlap
        return;
    }

    if(l<=start && end<=r)
    {
        // Complete overlap
        tree[node]=height;
        lazy[node]=height;
        return;
    }

    // Partial overlap
    int mid=(start+end)/2;
    int leftChild=node*2;
    int rightChild=node*2+1;

    // Push lazy values down
    pushDown(node, leftChild, rightChild);

    updateRange(leftChild, start, mid, l, r, height);
    updateRange(rightChild, mid+1, end, l, r, height);

    // Update current node value
    tree[node]=(tree[leftChild]>tree[rightChild]) ? tree[leftChild] : tree[rightChild];
}

// Get height array by traversing segment tree
void buildHeightArray(int node, int start, int end, long long heights[])
{
    if(start==end)
    {
        heights[start]=tree[node];
        return;
    }

    int mid=(start+end)/2;
    int leftChild=node*2;
    int rightChild=node*2+1;

    pushDown(node, leftChild, rightChild);

    buildHeightArray(leftChild, start, mid, heights);
    buildHeightArray(rightChild, mid+1, end, heights);
}

// ------------------- Main ---------------------

int main()
{
    int q;
    cin>>q;

    // Read queries
    for(int i=0;i<q;i++)
    {
        cin>>queryType[i];
        if(queryType[i]==0)
        {
            cin>>queryL[i]>>queryR[i]>>queryH[i];
            coordList[coordCount++]=queryL[i];
            coordList[coordCount++]=queryR[i];
        }
    }

    // Sort and compress coordinates
    quickSort(coordList, 0, coordCount-1);
    coordCount=removeDuplicates(coordList, coordCount);

    // Initialize segment tree and lazy array
    for(int i=0; i<MAX_TREE_SIZE; i++)
    {
        tree[i]=0;
        lazy[i]=-1;
    }

    // Process queries
    for(int i=0; i<q; i++)
    {
        if(queryType[i]==0)
        {
            int leftIndex=findIndex(coordList, coordCount, queryL[i]);
            int rightIndex=findIndex(coordList, coordCount, queryR[i]) - 1; // right is exclusive
            if(rightIndex>=leftIndex)
            {
                updateRange(1, 0, coordCount-2, leftIndex, rightIndex, queryH[i]);
            }
        }
        else
        {
            // Build array of heights
            static long long heights[MAX_COORDS];
            buildHeightArray(1, 0, coordCount-2, heights);

            // Print silhouette
            long long prevHeight=0;
            for(int j=0; j<=coordCount-2; j++)
            {
                if(heights[j]!=prevHeight)
                {
                    cout<<coordList[j]<< " "<<heights[j]<< endl;
                    prevHeight=heights[j];
                }
            }
            // At the very end, the skyline drops to 0
            if(prevHeight!=0)
            {
                cout<<coordList[coordCount-1]<<" 0"<<endl;
            }
        }
    }
    return 0;
}