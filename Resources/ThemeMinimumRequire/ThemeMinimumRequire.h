#pragma once
#include <unordered_set>
#include <string>

namespace cyanvne
{
	namespace resources
	{
		namespace themepack
		{
			class GetThemeDataRequirementsInterface
			{
			protected:
				GetThemeDataRequirementsInterface() = default;
			public:
				GetThemeDataRequirementsInterface(const GetThemeDataRequirementsInterface&) = delete;
                GetThemeDataRequirementsInterface& operator=(const GetThemeDataRequirementsInterface&) = delete;
                GetThemeDataRequirementsInterface(GetThemeDataRequirementsInterface&&) = delete;
                GetThemeDataRequirementsInterface& operator=(GetThemeDataRequirementsInterface&&) = delete;

				virtual const std::unordered_set<std::string>& getRequirements() const = 0;
                virtual bool isRequirementMet(const std::string& requirement) const = 0;
				virtual bool validateRequirements(const std::unordered_set<std::string>& requirements) const = 0;
				virtual double getThemeDataVersion() const = 0;

                virtual ~GetThemeDataRequirementsInterface() = default;
			};

			class ThemeDefaultMinimumRequire : public GetThemeDataRequirementsInterface
			{
			private:
				std::unordered_set<std::string> requirements_;
				double theme_data_version_ = 0.1;
			public:
				ThemeDefaultMinimumRequire()
				{
					requirements_ = {
						"BasicFont",
						"MainMenuLogo",
						"CreateCyanData",
						"CreateCyanTheme",
						"LoadCyanData",
						"LoadCyanTheme",
						"Setting",
						"Close",

						"DialogImg",
						"SaveProcess",
						"LoadProcess",
						"History",
						"SettingInGame",
						"HideDialogBox",
					};
				}

				const std::unordered_set<std::string>& getRequirements() const override
				{
                    return requirements_;
				}

				bool isRequirementMet(const std::string& requirement) const override
				{
                    return requirements_.find(requirement) != requirements_.end();
				}

				// Validation subset
				bool validateRequirements(const std::unordered_set<std::string>& requirements) const override
				{
					if (requirements_.size() > requirements.size())
					{
						return false;
					}

					for (const auto& element : requirements)
					{
						if (requirements_.find(element) == requirements_.end())
						{
							return false;
						}
					}

					return true;
				}

				double getThemeDataVersion() const override
				{
                    return theme_data_version_;	
				}

                ~ThemeDefaultMinimumRequire() override = default;
			};
		}
	}
}