#ifndef analytics_h
#define analytics_h

#include <string>
#include <vector>
#include <iostream>


using namespace std;

struct Results
{
  string detectorType;
  string descriptorType;
  string matcherType;
  string descriptorClass;
  string selectorType;

  vector<int> numKeypoints;
  vector<int> numMatches;

  vector<double> timeDetector;
  vector<double> timeDescriptor;

  vector<double> timeTotal;
};


void printAnalysis(Results results)
{
  cout << "\033[1;33m" << "ANALYTICS" << "\033[0m" << endl;
  cout << "Detector used: \t\t" << results.detectorType << endl;
  cout << "Descriptor used: \t" << results.descriptorType << endl;
  cout << "Matcher used: \t\t" << results.matcherType << endl;
  cout << "Selector used: \t\t" << results.selectorType << endl;

  int numKeypointsAverage = 0;
  int numMatchesAverage = 0;
  double timeDetectorAverage = 0.0;
  double timeDescriptorAverage = 0.0;
  double timeTotalAverage = 0.0;

  cout << "Frame\tnum kp\ttime det\tnum mat\ttime desc\ttime total" << endl;
  cout << results.numKeypoints.size() << endl;
  int numFrames = results.numKeypoints.size();
  for (int i = 0; i < numFrames; i++)
  {
    numKeypointsAverage += results.numKeypoints[i];
    timeDetectorAverage += results.timeDetector[i];
    timeDescriptorAverage += results.timeDescriptor[i];
    timeTotalAverage += results.timeTotal[i];
    if (i != 0)
      numMatchesAverage += results.numMatches[i];
    
    cout << setprecision(4) << fixed << i << "\t";
    cout << results.numKeypoints[i] << "\t" << 1000 * results.timeDetector[i] / 1.0 << "\t\t";
    cout << results.numMatches[i] << "\t" << 1000 * results.timeDescriptor[i] / 1.0 << "\t\t";
    cout << 1000 * results.timeTotal[i] / 1.0 << endl;
  }

  cout << setprecision(4) << fixed << "Avg" << "\t";
  cout << numKeypointsAverage/numFrames << "\t" << 1000 * (timeDetectorAverage/numFrames) / 1.0 << "\t\t";
  cout << numMatchesAverage/(numFrames-1) << "\t" << 1000 * (timeDescriptorAverage/numFrames) / 1.0 << "\t\t";
  cout << 1000 * (timeTotalAverage/numFrames) / 1.0 << endl;
}

#endif /* analytics_h */