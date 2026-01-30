#pragma once 


#include <string>
#include <array>
#include <glm/glm.hpp>
#include "../include/xresource_guid.h"
#include "../Serialization/ComponentRegistry.h"
namespace Engine {

	enum class TextAlignment {
		Left,
		Right,
		Center,
		Justified
	};

	struct TextComponent {

		static constexpr ComponentTypeID TypeID = ComponentTypeID::Text; 
		static constexpr const char* TypeName = "TextComponent";

		xresource::instance_guid ComponentGUID = 0;

		std::string text = "Text";
		std::string fontName = "Quantico-Regular"; //FONT SELECTION
		float fontSize = 24.0f; 
		std::array<float, 4> color = {1.0f,1.0f,1.0f,1.0f};

		//layout
		TextAlignment align = TextAlignment::Left;
		float lineSpacing = 1.0f;
		float letterSpacing = 0.0f;
		float maxWidth = 0.0f; 

		//runtime data
		bool isDirty = true;
		glm::vec4 bounds = { 0.0f,0.0f,0.0f,0.0f };
		

		//constructor 
		TextComponent() {
			ComponentGUID.GenerateGUID(); 
		}

		//setters
		void setText(const std::string& txt) {
			text = txt;
			isDirty = true;
		}

		void setFontSize(float size) {
			fontSize = glm::clamp(size, 1.0f, 200.f);
			isDirty = true;
		}

		void setColor(float r, float g, float b, float a) {
			color = {
				glm::clamp(r, 0.0f, 1.0f),
				glm::clamp(g, 0.0f, 1.0f),
				glm::clamp(b, 0.0f, 1.0f),
				glm::clamp(a, 0.0f, 1.0f)
			};
		}

		void setColor(const glm::vec4& col) {
			color = {
				glm::clamp(col.r, 0.0f, 1.0f),
				glm::clamp(col.g, 0.0f, 1.0f),
				glm::clamp(col.b, 0.0f, 1.0f),
				glm::clamp(col.a, 0.0f, 1.0f)
			};
		}

		void setAlignment(TextAlignment alignment) {
			align = alignment;
			isDirty = true;
		}

		void setLetterSpacing(float spacing) {
			letterSpacing = glm::clamp(spacing, -10.0f, 10.0f);
			isDirty = true;
		}

		void setFontName(const std::string& name) {
			fontName = name;
			isDirty = true;
		}

		//getters
		float getHeight() const {
			return bounds.w;
		}

		float getWidth() const {
			return bounds.z;
		}

		const std::string& getText() const { return text; }
		const std::string& getFontName() const { return fontName; }
		float getFontSize() const { return fontSize; }
		glm::vec4 getColor() const {
			return glm::vec4(color[0], color[1], color[2], color[3]);
		}

		TextAlignment getAlignment() const { return align;  }
		float getLetterSpacing() const { return letterSpacing; }
	};

}//end of namespace Engine