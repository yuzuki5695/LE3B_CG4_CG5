#include "AnimationPlayer.h"
#include <cassert>


Animation AnimationPlayer::LoadAnimationFile(const std::string& directoryPath, const std::string& filename) {
	Animation animation; // 今回作るアニメ―ション
	std::string filePath = directoryPath + "/" + filename;

	const aiScene* scene = importer.Readfile(filePath.c_str(), 0);
	assert(scene->mNumAnimations != 0);

}
