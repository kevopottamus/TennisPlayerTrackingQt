//#include <iostream>
//#include "MotionDetector.h"
//#include <opencv2/opencv.hpp>
//#include "SVMPlayerDetector.h"
//#include "ParticleFilter.h"
//#include <vector>
//#include <fstream>
//
//using namespace cv;
//
//int X1, Y1;
//int X2, Y2;
//Mat firstFrame;
//bool draw = false;
//int C=0;  
//std::vector<Rect> playerVector;
//Mat playerTemplate;
//Mat mask;
//
///// initialize SVMPlayerDetector
//SVMPlayerDetector svmPlayerDetector;	
///// video frame for getting negative patches
//cv::Mat g_videoFrame;
//
//void mouseEvent(int evt, int x, int y, int flags, void* param){
//	if(evt==CV_EVENT_LBUTTONDOWN){
//		X1 = x;
//		Y1 = y;
//		draw = true;
//		std::cout << "down" << x <<"," << y << std::endl;
//		C++;
//	}
//	if(evt==CV_EVENT_LBUTTONUP){
//		//rectangle(firstFrame, Point(X1, Y1), Point(x, y), Scalar(0, 0, 200), 1, CV_AA);
//		C++;
//		Rect rect(X1, Y1, x - X1, y - Y1);
//		playerVector.push_back(rect);
//		// set the bound box size for the SVMPlayerDetector
//		svmPlayerDetector.setBBSize(cv::Size(rect.width,rect.height));
//		firstFrame(rect).copyTo(playerTemplate);
//	}
//	if(C % 2 == 1 && evt == CV_EVENT_MOUSEMOVE){
//		if(draw){
//			Mat frame = firstFrame.clone();
//			rectangle(frame, Point(X1, Y1), Point(x, y), Scalar(0, 0, 200), 1, CV_AA);
//			imshow("test", frame);
//		}
//	}
//}
//
//cv::Point findPlayer(Mat& img)
//{
//
//	namedWindow("template");
//	imshow("template", playerTemplate);
//	namedWindow("frame");
//	imshow("frame", img);
//	waitKey(0);
//	/// Create the result matrix
//	int result_cols =  img.cols - playerTemplate.cols + 1;
//	int result_rows = img.rows - playerTemplate.rows + 1;
//	Mat result( result_cols, result_rows, CV_32FC1 );
//	/// Do the Matching and Normalize
//	matchTemplate( img, playerTemplate, result, CV_TM_SQDIFF_NORMED);
//	Mat resizeMask;
//	resize(mask, resizeMask, result.size());
//	normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );
//	namedWindow("result");
//	imshow("result", result);
//	waitKey(0);
//
//
//	/// Localizing the best match with minMaxLoc
//	double minVal; double maxVal; Point minLoc; Point maxLoc;
//	Point matchLoc;
//
//	minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, resizeMask);
//	// minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
//	matchLoc = minLoc;
//	std::cout << "max: " << maxVal << "\tmin: " << minVal << std::endl;
//	return matchLoc;
//}
//
//
//int main(int argc, char* argv[]){
//	std::vector<cv::Point> firstPlayerPos;
//	std::vector<cv::Point> secondPlayerPos;
//	std::string tennisCascade = "C:\\Users\\kevin\\Downloads\\opencv\\data\\hogcascades\\hogcascade_pedestrians.xml";
//	// read mask
//	Mat mask = imread("C:\\Users\\kevin\\Downloads\\train_tennis2\\mask.png");
//
//	// initialize cascade classifier.2
//	int frameCnt = 0;
//	int numFrame = 50;
//
//
//	for( int curPlayer = 0; curPlayer < 2; curPlayer ++)
//	{
//		CascadeClassifier tennis;
//		tennis.load(tennisCascade);
//		ParticleFilter pf1;
//		//MotionDetector motionDetector;
//
//		VideoCapture cap("C:/users/kevin/Videos/Youtube Videos/tennis5.mp4");
//		if(!cap.isOpened()){
//			return -1;
//		}
//
//		playerVector.clear();
//		cv::namedWindow("test");
//		setMouseCallback("test", mouseEvent, 0);
//		cap >> firstFrame;
//		imshow("test", firstFrame);
//		waitKey(0);
//
//		pf1.init(firstFrame, playerVector[0]);
//		pf1.draw(firstFrame);
//
//		bool isTracking = true;
//		cv::imshow("test", firstFrame);
//		waitKey(0);
//		std::vector<Rect> tennisbb;
//		int count = 5;
//		// store player centroid points
//		frameCnt = 0;
//		while(true && frameCnt < numFrame){
//			Mat frame;
//			cap >> frame;
//			frameCnt ++;
//			/*motionDetector.compute(frame);
//			Mat flow = motionDetector.get_flow();
//			Mat flow_magnitude = motionDetector.get_flow_magnitude();
//			motionDetector.draw_optical_flow(flow, frame, 8);
//			imshow("test", frame);
//			double minValue, maxValue;
//			minMaxLoc(flow_magnitude, &minValue, &maxValue);
//			flow_magnitude.convertTo(flow_magnitude, 1.0 / maxValue);
//			*/
//			//imshow("magnitude", flow_magnitude);
//			if(!isTracking){
//				--count;
//				if(count == 0 && !tennisbb.empty()){
//					std::vector<double> diffVector;
//					for(size_t i = 0; i < tennisbb.size(); ++i){
//						Mat diff;
//						absdiff(firstFrame(tennisbb[i]), frame(tennisbb[i]), diff);
//						Scalar smean = mean(diff);
//						double m = smean.val[0] + smean.val[1] + smean.val[2];
//						diffVector.push_back(m);
//					}
//					int maxIndex = std::max_element(diffVector.begin(), diffVector.end()) - diffVector.begin();
//					pf1.init(firstFrame, tennisbb[maxIndex]);
//					isTracking = true;
//					//rectangle(frame, tennisbb[maxIndex], Scalar(255,0,0), 2);
//					//imshow("test", frame);
//					//waitKey(0);
//				}
//			}else if(!pf1.update(frame)){
//				frame = frame & mask;
//				// particle filter loses tracking
//				tennis.detectMultiScale(frame, tennisbb);
//				firstFrame = frame.clone();
//				isTracking = false;
//				count = 5;
//			}
//			/// make a copy of current frame for negative sample generation
//			g_videoFrame = frame;
//			pf1.draw(frame);
//			imshow("test", frame);
//			cv::Point massCenter;
//			massCenter = pf1.mass_center(pf1.get_point_vector());
//
//			// save to the vector
//			if(0 == curPlayer){
//				firstPlayerPos.push_back(massCenter);
//				// add the patch of the bb to SVMPlayerDetector
//				svmPlayerDetector.getBBFromMassCenter(g_videoFrame,massCenter);
//			}
//			if(1== curPlayer){
//				secondPlayerPos.push_back(massCenter);
//			}
//			waitKey(30);
//			/*float offset = pf1.get_offset();
//			*/
//		}
//		if( 0 == curPlayer)
//		{
//			// generate negative samples
//			svmPlayerDetector.getNonPlayerPatches(g_videoFrame);
//		}
//	}
//
//	// display two players
//	VideoCapture cap("C:/users/kevin/Videos/Youtube Videos/tennis5.mp4");
//	if(!cap.isOpened()){
//		return -1;
//	}
//	std::string windowName = "showTwoPlayerPos";
//	cv::namedWindow( windowName );
//	// read frames
//	for( frameCnt = 0; frameCnt < numFrame; frameCnt++ )
//	{
//		// grab one frame from the video
//		Mat frame;
//		cap >> frame;
//		// draw the masscenter of two players on current frame
//		cv::Point fpPos = firstPlayerPos[frameCnt];
//		cv::Point spPos = secondPlayerPos[frameCnt];
//		// use cv::circle to draw
//		cv::circle(frame, fpPos, 5, cv::Scalar(220, 50, 40), 1, CV_AA);	
//		cv::circle(frame, spPos, 5, cv::Scalar(220, 50, 40), 1, CV_AA);	
//		// show the frame
//		cv::imshow( windowName, frame);
//		cv::waitKey(30);
//	}
//
//	// save the player positions to file
//	std::ofstream posFile;
//	std::string posFileName = "player_pos.txt";
//	posFile.open( posFileName, std::ofstream::out);
//	if( posFile.good())
//	{
//		// iterate through the position vectors
//		for(int i = 0; i < firstPlayerPos.size(); i++)
//		{
//			// get current position
//			cv::Point pt1 = firstPlayerPos[i];
//			cv::Point pt2 = secondPlayerPos[i];
//			// write the point coordinates to file
//			posFile << pt1.x << "\t" <<  pt1.y << "\t" <<  pt2.x << "\t" << pt2.y << std::endl;
//			
//		}
//	}
//	posFile.close();	
//	return 0;
//}