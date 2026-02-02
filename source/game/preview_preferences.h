#ifndef RME_GAME_PREVIEW_PREFERENCES_H_
#define RME_GAME_PREVIEW_PREFERENCES_H_

#include "game/outfit.h"
#include <string>
#include <vector>

struct FavoriteItem {
	Outfit outfit;
	std::string name;
	int speed;
	std::string label;
};

class PreviewPreferences {
public:
	PreviewPreferences();
	~PreviewPreferences();

	void load();
	void save();

	const Outfit& getOutfit() const {
		return current_outfit;
	}
	void setOutfit(const Outfit& outfit) {
		current_outfit = outfit;
	}

	int getSpeed() const {
		return current_speed;
	}
	void setSpeed(int speed) {
		current_speed = speed;
	}

	const std::string& getName() const {
		return current_name;
	}
	void setName(const std::string& name) {
		current_name = name;
	}

	const std::vector<FavoriteItem>& getFavorites() const {
		return favorites;
	}
	void setFavorites(const std::vector<FavoriteItem>& favs) {
		favorites = favs;
	}
	void addFavorite(const FavoriteItem& item) {
		favorites.push_back(item);
	}

private:
	Outfit current_outfit;
	int current_speed;
	std::string current_name;
	std::vector<FavoriteItem> favorites;
};

extern PreviewPreferences g_preview_preferences;

#endif
