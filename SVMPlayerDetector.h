#pragma once
#include <vector>
#include <opencv2/opencv.hpp>


struct RectScore{
	cv::Rect rect;
	double score;
};

class SVMPlayerDetector
{
protected:
	// storing bound boxes that contain player
	std::vector<cv::Rect> m_bbVect;
	// boundbox size
	cv::Size m_bbSize;
	// store patches containing player
	std::vector<cv::Mat> m_playerPatchVect;
	// store histogrames for player patches
	std::vector<cv::MatND> m_playerHistVect;
	// store non-player patches
	std::vector<cv::Mat> m_nonPlayerPatchVect;
	// store histogrames for non-player patches
	std::vector<cv::MatND> m_nonPlayerHistVect;
	/// store the accumulated histogram from all patches
	cv::MatND m_playerAccuPatch;
	cv::Mat m_bbPatch;
	cv::Mat m_patchFrame;
	/// define player zone
	cv::Rect m_playerZone;
	/// working frame, the frame to detect the player
	cv::Mat m_workFrame;
public:
	SVMPlayerDetector(cv::Size bbSize = cv::Size(30,80));
	void checkPatch(){
		cv::namedWindow("checkPatch");
		cv::imshow("checkPatch",this->m_bbPatch);
		cv::waitKey(5);
	}
	cv::Rect& getPlayerZone(){
		return this->m_playerZone;
	}
	cv::Mat& getWorkFrame(){
		return this->m_workFrame;
	}

	void setBBSize(cv::Size bbSize)
	{
		m_bbSize = bbSize;
	}
	void getNonPlayerPatches(cv::Mat videoFrame);
	void getBBFromMassCenter(cv::Mat videoFrame, cv::Point massCenter);
	void trainSVM();
	void detectFromHOG(cv::Mat videoFrame);
	cv::Rect detectByBackProject(cv::Mat videoFrame);
	cv::Rect detectByHistogramCompare(cv::Mat videoFrame, RectScore& bestRect);
	cv::Rect detectByRescalePatch(cv::Mat videoFrame, RectScore& bestRect);
	~SVMPlayerDetector(void);
protected:
	cv::MatND _getHistFromPatch(cv::Mat& patch);
	cv::MatND _getHistFromAllPatches();
	RectScore _histBackProject(cv::Mat& patch, cv::MatND& targetHist, cv::Size const& bb);
	double _compareTwoHistograms(cv::MatND& hist1, cv::MatND& hist2);
};

