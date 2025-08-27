#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstring>
using namespace cv;


// Helper function for printing progress bar
void printProgress(int current, int total)
{
    int barWidth=50;
    float progress=(float)current/total;

    std::cout << "[";
    int pos=barWidth*progress;
    for(int i=0;i<barWidth;i++)
    {
        if(i<pos)
        {
            std::cout << "=";
        }
        else if(i==pos)
        {
            std::cout << ">";
        }
        else
        {
            std::cout << " ";
        }
    }
    std::cout << "] " << int(progress*100.0) << " %\r";
    std::cout.flush();
}

// Computing energy matrix using dual gradient function
Mat calculateEnergy(const Mat& img)
{
    int H=img.rows, W=img.cols;
    Mat energy(H, W, CV_64F); // double precision
    for(int r=0;r<H;r++)
    {
        for(int c=0;c<W;c++)
        {
            int up=(r==0) ? H-1 : r-1; //wrap around if top row. Approach used in Princeton's seam carving assignment
            int down=(r==H-1) ? 0 : r+1;
            int left=(c==0) ? W-1 : c-1;
            int right=(c==W-1) ? 0 : c+1;

            Vec3b L=img.at<Vec3b>(r,left);
            Vec3b R=img.at<Vec3b>(r,right);
            Vec3b U=img.at<Vec3b>(up,c);
            Vec3b D=img.at<Vec3b>(down,c);

            double dx=pow(R[0]-L[0],2) + pow(R[1]-L[1],2) + pow(R[2]-L[2],2);
            double dy=pow(D[0]-U[0],2) + pow(D[1]-U[1],2) + pow(D[2]-U[2],2);

            energy.at<double>(r,c)=sqrt(dx+dy);
        }
    }
    return energy;
}

// Finding the least energy vertical seam
void findVerticalSeam(const Mat& energy, int* seam)
{
    int H=energy.rows, W=energy.cols;
    double** dp=new double*[H]; //Stores minimum cumulative energy to reach a particular row and column
    int** parent=new int*[H]; //Stores the column of the parent pixel from previous row

    for(int r=0;r<H;r++)
    {
        dp[r]=new double[W];
        parent[r]=new int[W];
    }

    // Initialization for top row
    for(int c=0;c<W;c++)
    {
        dp[0][c]=energy.at<double>(0,c);
        parent[0][c]=-1;
    }

    // Fill up the dp table
    for(int r=1;r<H;r++)
    {
        for(int c=0;c<W;c++)
        {
            double bestVal=dp[r-1][c];
            int bestCol=c;

            if(c>0 && dp[r-1][c-1]<bestVal)
            {
                bestVal=dp[r-1][c-1];
                bestCol=c-1;
            }
            if(c<W-1 && dp[r-1][c+1]<bestVal)
            {
                bestVal=dp[r-1][c+1];
                bestCol=c+1;
            }

            dp[r][c]=energy.at<double>(r,c)+bestVal;
            parent[r][c]=bestCol;
        }
    }

    // Finding the end of minimum seam in bottom row
    int minCol=0;
    double minVal=dp[H-1][0];
    for(int c=1;c<W;c++)
    {
        if(dp[H-1][c]<minVal)
        {
            minVal=dp[H-1][c];
            minCol=c;
        }
    }

    // Fill the parent array by backtracking
    int c=minCol;
    for(int r=H-1;r>=0;r--)
    {
        seam[r]=c;
        c=parent[r][c];
        if(c<0)
        {
            break;
        }
    }

    for(int r=0;r<H;r++)
    {
        delete[] dp[r];
        delete[] parent[r];
    }

    delete[] dp;
    delete[] parent;

}

// Removing a vertical seam
Mat removeVerticalSeam(const Mat& img, const int* seam)
{
    int H=img.rows, W=img.cols;
    Mat out(H, W-1, CV_8UC3); //8 bit unsigned, 3 channel: standard format for a colour image of B, G, R channels

    for(int r=0;r<H;r++)
    {
        int colRemoveIdx=seam[r];
        for(int c=0;c<colRemoveIdx;c++)
        {
            out.at<Vec3b>(r,c)=img.at<Vec3b>(r,c);
        }
        for(int c=colRemoveIdx+1;c<W;c++)
        {
            out.at<Vec3b>(r,c-1)=img.at<Vec3b>(r,c);
        }
    }
    return out;
}

// Carving multiple vertical seams until target width is reached
Mat carveVertical(const Mat& img, int targetW)
{
    Mat result=img.clone();
    int total=result.cols-targetW;

    for(int removed=0;removed<total;removed++)
    {
        Mat energy=calculateEnergy(result);
        int* seam=new int[result.rows];
        findVerticalSeam(energy,seam);
        result=removeVerticalSeam(result,seam);
        delete[]seam;

        printProgress(removed+1,total);
    }
    std::cout << std::endl;
    return result;
}

// Carving multiple horizontal seams
Mat carveHorizontal(Mat& img, int targetH)
{
    Mat result=img.clone();
    int total=result.rows-targetH; //number of seams to remove

    for(int removed=0;removed<total;removed++)
    {
        transpose(result,result);
        Mat energy=calculateEnergy(result);
        int* seam=new int[result.rows];
        findVerticalSeam(energy,seam);
        result=removeVerticalSeam(result,seam);
        delete[] seam;
        transpose(result,result);

        printProgress(removed+1,total);
    }
    std::cout << std::endl;
    return result;
}


int main()
{
    std::string path;
    std::cout << "Enter image path: ";
    std::cin >> path;

    Mat img=imread(path);
    if(img.empty())
    {
        std::cerr << "Error: Could not load image at " << path << std::endl;
        return 1;
    }

    int W, H;
    std::cout << "Enter the reduced width and height: ";
    std::cin >> W >> H;

    if(W<=0 || H<=0 || W>img.cols || H>img.rows)
    {
        std::cerr << "Invalid target dimensions" << std::endl;
        return 1;
    }

    Mat result=carveVertical(img,W);
    result=carveHorizontal(result,H);

    // Convert std::string path (input) to C-string for processing
    char cpath[512];
    strncpy(cpath, path.c_str(), sizeof(cpath));
    cpath[sizeof(cpath)-1]='\0';  // ensure null termination

    // Buffers for parts
    char folder[512], filename[256], name[256], ext[64];

    // 1. Find last slash
    const char* slash=strrchr(cpath, '/');
    #ifdef _WIN32
        if(!slash)
        {
            slash=strrchr(cpath, '\\'); // also handle Windows paths
        }
    #endif

    if(slash)
    {
        size_t folderLen=slash-cpath+1;  // include '/'
        strncpy(folder, cpath, folderLen);
        folder[folderLen]='\0';
        strcpy(filename, slash+1);
    }
    else
    {
        strcpy(folder, "");
        strcpy(filename, cpath);
    }

    // 2. Find last dot
    const char* dot=strrchr(filename, '.');
    if(dot)
    {
        size_t nameLen=dot-filename;
        strncpy(name, filename, nameLen);
        name[nameLen]='\0';
        strcpy(ext, dot);
    }
    else
    {
        strcpy(name, filename);
        strcpy(ext, "");
    }

    // 3. Build output path = folder + name + "_resized" + ext
    char outPath[1024];
    snprintf(outPath, sizeof(outPath), "%s%s_resized%s", folder, name, ext);

    // Save
    imwrite(outPath, result);
    std::cout<<"Saved resized image as: "<<outPath<<std::endl;

    return 0;
}