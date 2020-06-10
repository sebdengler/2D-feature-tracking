# 2D Feature Tracking

This project is largely inspired by the Udactiy Sensor Fusion Nanodegree course project (see https://github.com/udacity/SFND_2D_Feature_Tracking). It is used as a solution project submission for the 2D Feature Tracking module of the mentioned course.

The executable that is implemented in this project matches keypoints of successive camera frames to each other in order to track cars that are located in front of an ego vehicle over multiple frames. For this purpose different keypoint extractors and descriptors are implemented that can be compared to each other. The project makes use of the OpenCV library.

- The keypoint detection methods that are implemented are: Shi-Tomasi, Harris, FAST, BRISK, ORB, AKAZE and SIFT.
- The keypoint description methods that are implemented are: BRISK, BRIEF, ORB, FREAK, AKAZE and SIFT.
- For descriptor matching a brute force method or FLANN can be combined with nearest-neighbor- or kNN-matching.

The software was tested on Ubuntu 18.04.


## Usage Instructions

### Install Dependencies

The following guide was used to install OpenCV (4.3) on Ubuntu 18.04:

* https://linuxize.com/post/how-to-install-opencv-on-ubuntu-18-04/

Various other tutorials can be found on the internet for installing OpenCV on other operating systems.

Additional dependecies are cmake (>= 2.8), make (>= 4.1) and gcc/g++ (>= 5.4).


### Build the Project

```
git clone
cd 2d-feature tracking
mkdir build && cd build
cmake ..
make
```

### Run the Executable

```
./2D_feature_tracking
```


## Analysis

In order to evaluate different detector/descriptor pairs and give a recommendation on which pair to use, different metrics were defined. The performances of the detectors/descriptors based on these metrics are shown in the following tables.

### Keypoint Detection

Number of detected keypoints per frame and total average:

| Frames | Shi-Tomasi | Harris | FAST | BRISK | ORB | AKAZE | SIFT |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 0       | 127 | 17 | 107 | 254 |  91 | 162 | 137 |
| 1       | 120 | 14 | 104 | 274 | 102 | 157 | 131 |
| 2       | 123 | 18 | 107 | 276 | 106 | 159 | 121 |
| 3       | 120 | 22 | 112 | 275 | 113 | 154 | 135 |
| 4       | 120 | 25 | 108 | 293 | 109 | 162 | 134 |
| 5       | 115 | 44 | 114 | 275 | 124 | 163 | 139 |
| 6       | 114 | 17 |  96 | 289 | 129 | 173 | 136 |
| 7       | 125 | 30 | 102 | 268 | 127 | 175 | 147 |
| 8       | 112 | 26 | 105 | 260 | 124 | 175 | 156 |
| 9       | 113 | 33 | 102 | 250 | 125 | 175 | 135 |
| Average | 118 | 24 | 105 | 271 | 115 | 165 | 137 |


Average time for detection in ms:

| Shi-Tomasi | Harris | FAST | BRISK | ORB | AKAZE | SIFT |
| --- | --- | --- | --- | --- | --- | --- |
| 27.39 | 24.01 | 1.65 | 50.01 | 14.48 | 96.71 | 130.12 |


### Keypoint Matching

Average number of matches for all detector/descriptor combinations:

| Detector/Desriptor| BRISK | BRIEF | ORB | FREAK | AKAZE | SIFT |
| --- | --- | --- | --- | --- | --- | --- |
| **Shi-Tomasi** | 85 | 105 | 101 | 85 | - | 104 |
| **Harris** | 15 | 19 | 17 | 16 | - | 18 |
| **FAST** | 74 | 89 | 89 | 72 | - | 84 |
| **BRISK** | 171 | 186 | 164 | 166 | - | 181 |
| **ORB** | 82 | 60 | 83 | 46 | - | 84 |
| **AKAZE** | 133 | 139 | 130 | 131 | 138 | 141 |
| **SIFT** | 65 | 77 | - | 65 | - | 88 |

Average time required for description with corresponding detector in ms:

| Detector/Desriptor| BRISK | BRIEF | ORB | FREAK | AKAZE | SIFT |
| --- | --- | --- | --- | --- | --- | --- |
| **Shi-Tomasi** | 2.63 | 2.35 | 7.20 | 59.41 | - | 33.95 |
| **Harris** | 1.42 | 1.88 | 6.33 | 67.11 | - | 23.36 |
| **FAST** | 1.77 | 1.37 | 11.80 | 57.17 | - | 25.00 |
| **BRISK** | 2.72 | 1.21 | 12.64 | 39.01 | - | 26.81 |
| **ORB** | 1.28 | 0.68 | 13.74 | 39.74 | - | 39.73 |
| **AKAZE** | 2.04 | 1.01 | 9.71 | 37.97 | 50.24 | 20.57 |
| **SIFT** | 1.38 | 0.88 | - | 41.91 | - | 68.66 |


### Evaluation

Some general remarks:
- The AKAZE descriptor works only when also using AKAZE for finding keypoints.
- The ORB descriptor does not work together with the SIFT detector.
- The SIFT descriptor only works with FLANN-based matching, not with the Brute Force Matching approach that was used with all other descriptors.

Performance results:
- The Harris detector, as it was parameterized here, finds the least amount of keypoints (24 in average), whereas BRISK finds the most (271). All other detectors are able to find a similar amount of keypoints betwenn 105 and 165 per frame.
- The FAST method is by far the fastest method for keypoint detection, whereas the SIFT method is by far the slowest. AKAZE and BRISK are relatively slow as well, whereas the execution time of Shi-Tomasi, Harris and ORB lies in between.
- Most descriptors are able to find the most matches in combination with BRISK and AKAZE detectors. It can be observed that the number of matches found depends very much on the number of keypoints that were detected by the detector and given as an input to the descriptor.
- Comparing the execution time of each descriptor it can be observed that FREAK, SIFT and AKAZE are relatively slow, whereas BRISK and BRIEF are very fast. The execution time of the descriptors are largely independent of the keypoint detection method that they are combined with.

A good combination of detector and descriptor should have a realatively short runtime while being able to find a large number of matches. Based on this, the following detector/descriptor pairs are recommended:
- For a good relation between speed and the number of matches use **FAST-BRIEF**.
- If the number of matches is more important than speed use **BRISK-BRIEF**.
- As a third alternative with high speed and a realtively large number of matches use **BRISK-BRISK**.


Be aware that a high number of matches does not necessarily result in a high number of correct matches since some matches could be false positives. However, analyzing the ratio of false positives to total matches would require a deeper analysis that would extend the purpose of this project.