/* INCLUDES FOR THIS PROJECT */
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <deque>
#include <cmath>
#include <limits>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

#include "dataStructures.h"
#include "analytics.h"
#include "matching2D.hpp"

using namespace std;

/* MAIN PROGRAM */
int main(int argc, const char *argv[])
{

  /* INIT VARIABLES AND DATA STRUCTURES */

  // data location
  string dataPath = "../";

  // camera
  string imgBasePath = dataPath + "images/";
  string imgPrefix = "KITTI/2011_09_26/image_00/data/000000"; // left camera, color
  string imgFileType = ".png";
  int imgStartIndex = 0; // first file index to load (assumes Lidar and camera names have identical naming convention)
  int imgEndIndex = 9;   // last file index to load
  int imgFillWidth = 4;  // no. of digits which make up the file index (e.g. img-0001.png)

  // misc
  int dataBufferSize = 2;      // no. of images which are held in memory (ring buffer) at the same time
  deque<DataFrame> dataBuffer; // list of data frames which are held in memory at the same time
  bool bVis = false;           // visualize results
  bool bVisKp = false;         // visualize keypoints

  // detection, description and matching methods
  string detectorType = "SHITOMASI";     // SHITOMASI, HARRIS, FAST, BRISK, ORB, AKAZE, SIFT
  string descriptorType = "FREAK";     // BRISK, BRIEF, ORB, FREAK, AKAZE, SIFT
  string matcherType = "MAT_BF";   // MAT_BF, MAT_FLANN
  string descriptorClass = "DES_BINARY"; // DES_BINARY, DES_HOG
  string selectorType = "SEL_KNN";    // SEL_NN, SEL_KNN

  Results results;
  results.detectorType = detectorType;
  results.descriptorType = descriptorType;
  results.matcherType = matcherType;
  results.descriptorClass = descriptorClass;
  results.selectorType = selectorType;

  bool bStoreResults = true;
  bool bFocusOnVehicle = true;
  bool bLimitKpts = false;


  /* MAIN LOOP OVER ALL IMAGES */

  for (size_t imgIndex = 0; imgIndex <= imgEndIndex - imgStartIndex; imgIndex++)
  {
    double timeOverall = (double)cv::getTickCount();
    
    /* LOAD IMAGE INTO BUFFER */

    // assemble filenames for current index
    ostringstream imgNumber;
    imgNumber << setfill('0') << setw(imgFillWidth) << imgStartIndex + imgIndex;
    string imgFullFilename = imgBasePath + imgPrefix + imgNumber.str() + imgFileType;

    // load image from file and convert to grayscale
    cv::Mat img, imgGray;
    img = cv::imread(imgFullFilename);
    cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);

    // push image into data frame buffer
    DataFrame frame;
    frame.cameraImg = imgGray;
    dataBuffer.push_back(frame);
    // remove old images from the buffer
    while (dataBuffer.size() > dataBufferSize)
      dataBuffer.pop_front();

    cout << "#1 : LOAD IMAGE INTO BUFFER done" << endl;

    /* DETECT IMAGE KEYPOINTS */

    // extract 2D keypoints from current image
    vector<cv::KeyPoint> keypoints; // create empty feature list for current image
    double timeDetector;

    // available keypoint detection methods are: SHITOMASI, HARRIS, FAST, BRISK, ORB, AKAZE, SIFT
    if (detectorType.compare("SHITOMASI") == 0)
    {
      detKeypointsShiTomasi(keypoints, imgGray, timeDetector, bVisKp);
    }
    else if (detectorType.compare("HARRIS") == 0)
    {
      detKeypointsHarris(keypoints, imgGray, timeDetector, bVisKp);
    }
    else
    {
      detKeypointsModern(keypoints, imgGray, timeDetector, detectorType, bVisKp);
    }

    // only keep keypoints on the preceding vehicle
    cv::Rect vehicleRect(535, 180, 180, 150);
    if (bFocusOnVehicle)
    {
      for (auto kIt = keypoints.begin(); kIt != keypoints.end(); )
      {
        if (kIt->pt.x < vehicleRect.tl().x || kIt->pt.x > vehicleRect.br().x ||
            kIt->pt.y < vehicleRect.tl().y || kIt->pt.y > vehicleRect.br().y)
        {
          kIt = keypoints.erase(kIt);
        }
        else
        {
          ++kIt;
        }
        
      }
    }

    // optional : limit number of keypoints
    if (bLimitKpts)
    {
      int maxKeypoints = 50;

      if (detectorType.compare("SHITOMASI") == 0)
      { // there is no response info, so keep the first 50 as they are sorted in descending quality order
        keypoints.erase(keypoints.begin() + maxKeypoints, keypoints.end());
      }
      cv::KeyPointsFilter::retainBest(keypoints, maxKeypoints);
      cout << " NOTE: Keypoints have been limited!" << endl;
    }

    // push keypoints and descriptor for current frame to end of data buffer
    (dataBuffer.end() - 1)->keypoints = keypoints;
    cout << "#2 : DETECT KEYPOINTS done" << endl;

    /* EXTRACT KEYPOINT DESCRIPTORS */

    // avaible descriptors are: BRISK, BRIEF, ORB, FREAK, AKAZE, SIFT
    cv::Mat descriptors;
    double timeDescriptor;
    descKeypoints((dataBuffer.end() - 1)->keypoints, (dataBuffer.end() - 1)->cameraImg, descriptors, timeDescriptor, descriptorType);

    // push descriptors for current frame to end of data buffer
    (dataBuffer.end() - 1)->descriptors = descriptors;

    cout << "#3 : EXTRACT DESCRIPTORS done" << endl;

    if (dataBuffer.size() > 1) // wait until at least two images have been processed
    {

      /* MATCH KEYPOINT DESCRIPTORS */

      vector<cv::DMatch> matches;

      matchDescriptors((dataBuffer.end() - 2)->keypoints, (dataBuffer.end() - 1)->keypoints,
                       (dataBuffer.end() - 2)->descriptors, (dataBuffer.end() - 1)->descriptors,
                       matches, descriptorClass, matcherType, selectorType);

      // store matches in current data frame
      (dataBuffer.end() - 1)->kptMatches = matches;

      cout << "#4 : MATCH KEYPOINT DESCRIPTORS done" << endl;

      if (bStoreResults)
      {
        results.numKeypoints.push_back(keypoints.size());
        results.numMatches.push_back(matches.size());
        results.timeDetector.push_back(timeDetector);
        results.timeDescriptor.push_back(timeDescriptor);

        timeOverall = ((double)cv::getTickCount() - timeOverall) / cv::getTickFrequency();
        results.timeTotal.push_back(timeOverall);
      }
      
      // visualize matches between current and previous image
      // bVis = true;
      if (bVis)
      {
        cv::Mat matchImg = ((dataBuffer.end() - 1)->cameraImg).clone();
        cv::drawMatches((dataBuffer.end() - 2)->cameraImg, (dataBuffer.end() - 2)->keypoints,
                (dataBuffer.end() - 1)->cameraImg, (dataBuffer.end() - 1)->keypoints,
                matches, matchImg,
                cv::Scalar::all(-1), cv::Scalar::all(-1),
                vector<char>(), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

        string windowName = "Matching keypoints between two camera images";
        cv::namedWindow(windowName, 7);
        cv::imshow(windowName, matchImg);
        cout << "\033[1;33m" << "Press key to continue to next image" << "\033[0m" << endl;
        cv::waitKey(0); // wait for key to be pressed
      }
      // bVis = false;
    }
    else
    {
      if (bStoreResults)
      {
        results.numKeypoints.push_back(keypoints.size());
        results.numMatches.push_back(0);
        results.timeDetector.push_back(timeDetector);
        results.timeDescriptor.push_back(timeDescriptor);

        timeOverall = ((double)cv::getTickCount() - timeOverall) / cv::getTickFrequency();
        results.timeTotal.push_back(timeOverall);
      }
    }
    

  } // eof loop over all images

  if (bStoreResults)
  {
    printAnalysis(results);
  }

  return 0;
}
