#include "SVMPlayerDetector.h"
#include <stdlib.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


SVMPlayerDetector::SVMPlayerDetector(cv::Size bbSize):m_bbSize(bbSize)
{

}


SVMPlayerDetector::~SVMPlayerDetector(void)
{

}

cv::MatND SVMPlayerDetector::_getHistFromPatch(cv::Mat& patch){
	// each channel has 50 levels
	int histSize[] = {50,50,50};
	// each channel range from 0 to 256 (excluded)
	/// red changel range
	float rranges[] = { 0, 256 };
	float granges[] = { 0, 256 };
	float branges[] = { 0, 256 };
	const float* ranges[] = { rranges, granges, branges };
	cv::MatND hist;
	// we compute the histogram from the 0-th to 2-th channel
	int channels[] = {0, 1, 2};
	calcHist( &patch, 1, channels, cv::Mat(), // do not use mask
		hist, 3, histSize, ranges,
		true, // the histogram is uniform
		false );
	/// return the histogram obtained
	return hist;
}

RectScore SVMPlayerDetector::_histBackProject(cv::Mat& patch, cv::MatND& targetHist,cv::Size const& bbSize){
	// stay with RGB space
	/// back projection to the target histogram
		int histSize[] = {50,50,50};
	// each channel range from 0 to 256 (excluded)
	/// red changel range
	float rranges[] = { 0, 256 };
	float granges[] = { 0, 256 };
	float branges[] = { 0, 256 };
	int channels[] = {0,1,2};
	const float* ranges[] = { rranges, granges, branges };
	cv::MatND outputHist;
	cv::calcBackProject(&patch,1,channels,targetHist,outputHist,ranges);
	cv::namedWindow("patch");
	cv::namedWindow("patch_bp");
	cv::imshow("patch_bp",outputHist);
	cv::imshow("patch",patch);
	cv::waitKey(0);
	// scan the patch and get the maximum response
	int patchH = patch.size().height;
	int patchW = patch.size().width;
	float maxSum = 0;
	int maxI, maxJ;
	for(int i = 0; i < patchH - bbSize.height; i++)
		for(int j = 0; j < patchW - bbSize.width; j++){
			/// sum at current bb at outputHist
			cv::Rect roi(j,i,bbSize.width,bbSize.height);
			cv::Scalar tmpSum = cv::sum(patch(roi));
			if(tmpSum[0] > maxSum){
				maxSum = tmpSum[0];
				maxI = i;
				maxJ = j;
			}
	}
	RectScore matchScore;
	matchScore.rect = cv::Rect(maxJ,maxI,bbSize.width,bbSize.height);
	matchScore.score = maxSum;
	return matchScore;
}

cv::MatND SVMPlayerDetector::_getHistFromAllPatches()
{
	// each channel has 50 levels
	int histSize[] = {50,50,50};
	// each channel range from 0 to 256 (excluded)
	/// red changel range
	float rranges[] = { 0, 256 };
	float granges[] = { 0, 256 };
	float branges[] = { 0, 256 };
	int channels[] = {0,1,2};
	const float* ranges[] = { rranges, granges, branges };
	
	for(int i = 0; i < this->m_playerPatchVect.size(); i++)
	{
		cv::Mat& patch = m_playerPatchVect[i];
		cv::namedWindow("testPatch");
		cv::imshow("testPatch",patch);
		cv::waitKey(0);

		calcHist( &patch, 1, channels, cv::Mat(), // do not use mask
			this->m_playerAccuPatch, 3, histSize, ranges,
			true, // the histogram is uniform
			true /// accumulate through all patches
			);

	}
	return this->m_playerAccuPatch;
}


void mouseEventBB(int evt, int x, int y, int flags, void* param){
	SVMPlayerDetector* detector = static_cast<SVMPlayerDetector*>(param);
	if(evt==CV_EVENT_LBUTTONDOWN){
		/// set the top left corner of the player zone
		detector->getPlayerZone().x = x;
		detector->getPlayerZone().y = y;
		detector->getPlayerZone().width = 200;
		detector->getPlayerZone().height = 200;
		/// draw the zone on the working frame
		cv::Mat tmpClone = detector->getWorkFrame().clone();
		cv::rectangle(tmpClone,detector->getPlayerZone(),cv::Scalar(255,255,255));
		cv::imshow("scanPatch",tmpClone);
	}
	if(evt==CV_EVENT_LBUTTONUP){
		//rectangle(firstFrame, Point(X1, Y1), Point(x, y), Scalar(0, 0, 200), 1, CV_AA);
	}
}



bool rectScoreCompare(RectScore rect1, RectScore rect2){
	return rect1.score > rect2.score;
}


cv::Rect SVMPlayerDetector::detectByBackProject(cv::Mat videoFrame){
	cv::HOGDescriptor hog;
	//assert(hog.load("C:\\Program Files\\opencv\\data\\hogcascades\\hogcascade_pedestrians.xml"));
	hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
	std::vector<cv::Rect> foundRects;
	/// to gray-scale image
	cv::Mat grayFrame;
	cv::cvtColor(videoFrame, grayFrame, CV_BGR2GRAY);
	hog.detectMultiScale(grayFrame,foundRects);
	/// display the rectangles
	cv::Mat frameClone = videoFrame.clone();
	cv::namedWindow("hog_result");
	/// visualize all found bounding boxes
		for(std::size_t i = 0; i < foundRects.size(); i++){
		cv::rectangle(frameClone,foundRects[i],cv::Scalar(255,255,0),2);
		cv::imshow("hog_result",frameClone);
	}
	cv::waitKey(0);
	std::vector<RectScore> foundRectScores;
	this->_getHistFromAllPatches();
	for(std::size_t i = 0; i < foundRects.size(); i++){
		/// set this rectangle as the bounding box for the player
		this->m_playerZone = foundRects[i];
		RectScore matchRect;
		cv::Mat tmpPatch = videoFrame(foundRects[i]);
		matchRect = _histBackProject(tmpPatch,m_playerAccuPatch,this->m_bbSize);
		foundRectScores.push_back(matchRect);
		/// output the score 
		//std::cout << "best match score:" << matchRect.score << std::endl;
	}

	//std::sort(foundRectScores.begin(),foundRectScores.end());
	cv::namedWindow("bp_result");
	std::size_t bestIdx = 0;
	float bestScore = 0;
	for(std::size_t i = 0; i < foundRectScores.size();i++){
		RectScore& matchRect = foundRectScores[i];
		std::cout << "best match score:" << matchRect.score << std::endl;
		if( i == 0 || matchRect.score > bestScore){
			bestIdx = i;
			bestScore = matchRect.score;
		}
	}
	/// draw the best match rect
	cv::Mat bpFrame = videoFrame.clone();
	cv::rectangle(bpFrame,foundRectScores[bestIdx].rect,cv::Scalar(255,255,255),2);
	cv::imshow("bp_result",bpFrame);
	cv::waitKey(0);
	cv::destroyWindow("hog_result");
	cv::destroyWindow("bp_result");
	return foundRectScores[bestIdx].rect;
}

void SVMPlayerDetector::detectFromHOG(cv::Mat videoFrame){
	static bool debug = false;
	cv::HOGDescriptor hog;
	//assert(hog.load("C:\\Program Files\\opencv\\data\\hogcascades\\hogcascade_pedestrians.xml"));
	hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
	std::vector<cv::Rect> foundRects;
	/// to gray-scale image
	cv::Mat grayFrame;
	cv::cvtColor(videoFrame, grayFrame, CV_BGR2GRAY);
	hog.detectMultiScale(grayFrame,foundRects,0);
	/// display the rectangles
	cv::Mat frameClone = videoFrame.clone();
	cv::namedWindow("hog_result");
	/// visualize all found bounding boxes
		for(std::size_t i = 0; i < foundRects.size(); i++){
		cv::rectangle(frameClone,foundRects[i],cv::Scalar(255,255,0),2);
		cv::imshow("hog_result",frameClone);
	}
	cv::waitKey(0);
	this->_getHistFromAllPatches();
	std::vector<RectScore> foundRectScores;
	for(std::size_t i = 0; i < foundRects.size(); i++){
		/// set this rectangle as the bounding box for the player
		this->m_playerZone = foundRects[i];
		RectScore matchRect;
		//this->detectByHistogramCompare(videoFrame, matchRect);
		this->detectByRescalePatch(videoFrame,matchRect);
		foundRectScores.push_back(matchRect);
		/// output the score 
		std::cout << "best match score:" << matchRect.score << std::endl;
	}

	std::size_t bestIdx = 0;
	float bestScore = 0;
	for(std::size_t i = 0; i < foundRectScores.size();i++){
		RectScore& matchRect = foundRectScores[i];
		std::cout << "best match score:" << matchRect.score << std::endl;
		if( i == 0 || matchRect.score > bestScore){
			bestIdx = i;
			bestScore = matchRect.score;
		}
	}
	/// draw the best match rect
	cv::rectangle(frameClone,foundRectScores[bestIdx].rect,cv::Scalar(255,255,255),2);
	cv::imshow("hog_result",frameClone);
	cv::waitKey(0);
	cv::destroyWindow("hog_result");

}

cv::Rect SVMPlayerDetector::detectByRescalePatch(cv::Mat videoFrame, RectScore& bestRect){
	// get the player patch
	cv::Mat playerPatch = videoFrame(m_playerZone);
	// rescale to the bounding box size
	cv::Mat rescaledPatch;
	cv::resize(playerPatch,rescaledPatch,this->m_bbSize);
	// calculate the histogram for tmpPatch (non-accumulate histogram)
	cv::MatND tmpHist = this->_getHistFromPatch(rescaledPatch);
	// compare tmpHist to player histogram
	double tmpScore = this->_compareTwoHistograms(this->m_playerAccuPatch, tmpHist);
	bestRect.score = tmpScore;
	bestRect.rect = m_playerZone;
	return m_playerZone;
}

cv::Rect SVMPlayerDetector::detectByHistogramCompare(cv::Mat videoFrame, RectScore& bestRect)
{
	static bool debug = false;
	// get the accumulated histogram for player patches
	this->m_workFrame = videoFrame.clone();
	cv::Size frameSize = videoFrame.size();
	// scan the frame from left-top corner to bottom-right corner
	int stepWidth = 5;
	double bestMatchScore = 0;
	int bestI, bestJ;
	cv::namedWindow("scanPatch");
	// bind the mouse event for selecting player zone
	//cv::setMouseCallback("scanPatch",mouseEventBB,this);
	cv::Mat frameClone = videoFrame.clone();
	cv::imshow("scanPatch",frameClone);
	//cv::waitKey(0);

	int startY = this->m_playerZone.y;
	int startX = this->m_playerZone.x;
	int endY, endX;
	if(m_playerZone.height < m_bbSize.height){
		m_bbSize.height = m_playerZone.height - 1;
	}
	endY = startY + this->m_playerZone.height - m_bbSize.height;
	
	if(m_playerZone.width < m_bbSize.width){
		m_bbSize.width = m_playerZone.width - 1;
	}
	endX = startX + this->m_playerZone.width - m_bbSize.width;
	std::vector<RectScore> rectScores;

	for(int i = startY; i < endY; i += stepWidth)
	{
		for(int j = startX; j < endX; j += stepWidth)
		{
			// get the boundbox with (i,j) as top-left corner
			cv::Rect tmpRect(j,i ,m_bbSize.width, m_bbSize.height);
			cv::Mat tmpPatch = videoFrame(tmpRect);
			// calculate the histogram for tmpPatch (non-accumulate histogram)
			cv::MatND tmpHist = this->_getHistFromPatch(tmpPatch);
			// compare tmpHist to player histogram
			double tmpScore = this->_compareTwoHistograms(this->m_playerAccuPatch, tmpHist);
			cv::Mat frameClone = videoFrame.clone();
			cv::rectangle(frameClone,tmpRect,CV_RGB(255,255,255),1);
			cv::imshow("scanPatch",frameClone);
			cv::waitKey(1);
			std::cout << tmpScore << std::endl;
			RectScore tmpRectScore;
			tmpRectScore.rect = tmpRect;
			tmpRectScore.score = tmpScore;
			rectScores.push_back(tmpRectScore);
			if( tmpScore > bestMatchScore)
			{
				bestMatchScore = tmpScore;
				bestI = i;
				bestJ = j;
			}
		}
	}

	std::sort(rectScores.begin(), rectScores.end(),rectScoreCompare);

	cv::Rect matchBB(bestJ,bestI,m_bbSize.width,m_bbSize.height);
	/// visualize the matched bounding box
	cv::namedWindow("match_bb");
	const unsigned int numTop = 3;
	float avgX=0, avgY=0;
	bestRect = rectScores[0];
	if(debug){

	for(int i = 0; i < rectScores.size() && i < numTop; i++)
	{
		RectScore tmpRectScore = rectScores[i];
		cv::Rect tmpRect = tmpRectScore.rect;
		avgX = avgX + tmpRect.x;
		avgY = avgY + tmpRect.y;
		std::cout << "rect score:" << tmpRectScore.score << std::endl;
		cv::Mat frameClone = videoFrame.clone();
		cv::rectangle(frameClone,tmpRectScore.rect,cv::Scalar(255,0,0),2);
		cv::imshow("match_bb",frameClone);
		if(i == numTop -1 ){
			avgX /= numTop;
			avgY /= numTop;
			// display the average bound box
			frameClone = videoFrame.clone();
			cv::Rect avgRect = tmpRect;
			avgRect.x = avgX;
			avgRect.y = avgY;
			cv::rectangle(frameClone,avgRect,cv::Scalar(0,255,0),2);
			cv::imshow("match_bb",frameClone);
		}
		cv::waitKey(0);
	}
	}
	cv::destroyWindow("scanPatch");
	cv::destroyWindow("match_bb");
	/// 
	return matchBB;
}

double SVMPlayerDetector::_compareTwoHistograms(cv::MatND& hist1, cv::MatND& hist2)
{
	// calculate the correlation (similarity) between hist1 and hist2
	return cv::compareHist(hist1,hist2, CV_COMP_INTERSECT ); /// use correlation as metric
}

/**
* train SVM classifier based on the positive and negative patches prepared
* the features extracted from patches is RGB color histogram
*/
void SVMPlayerDetector::trainSVM()
{
	// prepare histogram for player patches
	for(int i = 0; i < this->m_playerPatchVect.size(); i++)
	{
		cv::MatND tmpHist = this->_getHistFromPatch(m_playerPatchVect[i]);
		// store the histogram for current patch
		this->m_playerHistVect.push_back(tmpHist);
	}

	// prepare histogram for non-player patches
	for(int i = 0; i < this->m_nonPlayerPatchVect.size(); i++)
	{
		cv::MatND tmpHist = this->_getHistFromPatch(m_nonPlayerPatchVect[i]);
		// store the histogram for current patch
		this->m_nonPlayerHistVect.push_back(tmpHist);
	}
	// train the SVM classifier
	/// prepare feature matrix for positive samples
	/// 
}

/**
* generate negative sample patches by randomly sampling from the frame
* Arguments:
*	videoFrame: the video frame from which the samples are generated
*/
void SVMPlayerDetector::getNonPlayerPatches(cv::Mat videoFrame)
{
	/// prepare the same number of samples
	int numSamples = this->m_playerPatchVect.size();
	// generate random image patches
	int frameWidth = videoFrame.size().width;
	int frameHeight = videoFrame.size().height;
	// range for the samples
	int xMin = m_bbSize.width;
	int xMax = frameWidth - m_bbSize.width;
	int yMin = m_bbSize.height;
	int yMax = frameHeight - m_bbSize.height;

	for(int i = 0; i < numSamples; i++)
	{
		// generate random position
		// generate x in the range ( m_bbSize.width/2 + 1, frameWidth - m_bbSize.width/2 - 1)
		int randX = rand() % (xMax - xMin);
		randX += xMin;
		int randY = rand() % (yMax - yMin);
		randY += yMin;
		// extract the patch
		cv::Rect tmpRect(randX,randY,m_bbSize.width,m_bbSize.height);
		cv::Mat tmpPatch = videoFrame(tmpRect);
		// save to the non-player patch list
		this->m_nonPlayerPatchVect.push_back(tmpPatch);
		/// visualize the patches for verification purpose
		cv::namedWindow("negbb");
		cv::imshow("negbb",tmpPatch);
		cv::waitKey(30);
	}
}


/**
* get the bound box based on the mass center computed from Partical Filter algorithm
* Arguments:
*	videoFrame: current frame
*	massCenter: center of the particles
*/
void SVMPlayerDetector::getBBFromMassCenter(cv::Mat videoFrame,cv::Point massCenter)
{
	int leftPos = massCenter.x - this->m_bbSize.width / 2;
	int topPos = massCenter.y - this->m_bbSize.height / 2;
	/// judge whether current bound box exceeds the frame border
	if( leftPos <0 || topPos < 0 || leftPos + m_bbSize.width >= videoFrame.size().width || topPos + m_bbSize.height >= videoFrame.size().height )
	{
		return;
	}

	this->m_patchFrame = videoFrame.clone();
	// assign to tmpRect
	// get the top-left corner of the bound box from massCenter
	cv::Rect tmpRect(leftPos, topPos, m_bbSize.width, m_bbSize.height);
	// add to the bounding box vector
	this->m_bbVect.push_back(tmpRect);
	// get a patch from current frame for tmpRect
	cv::Mat tmpPatch = m_patchFrame(tmpRect);
	// cv::rectangle(videoFrame,tmpRect,cv::Scalar(255,255,255),1);
	this->m_playerPatchVect.push_back(tmpPatch);
	this->m_bbPatch = tmpPatch;
}
