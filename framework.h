#ifndef VISCG_CG_FRAMEWORK_H
#define VISCG_CG_FRAMEWORK_H

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#define GLEW_STATIC
#include "GL/glew.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
namespace viscg {
    //-------------------------------------------------------------------------------------------------
    // creates a single window with the specified width, height and caption text
    // optionaly the number of FSAA samples, window's position and whether debug context schould be used are specified
    bool OpenWindow(int width, int height, const std::string& caption, int fsaahint = 4, int x = 150, int y = 250, bool debugContext = true);

    //-------------------------------------------------------------------------------------------------
    // swaps the front-back buffers
    void SwapBuffers();

    //-------------------------------------------------------------------------------------------------
    // returns false when the window is closed.. i.e. when the main rendering loop should be leaved
    bool ProcessAllMesages(float sleep = 0);
    bool HasActiveWindows();

    //-------------------------------------------------------------------------------------------------
    // retrives the current desktop resolution
    void DesktopResolution(int& horizontal, int& vertical);

    //-------------------------------------------------------------------------------------------------
    // retrieves the current window resolution
    void WindowResolution(int& horizontal, int& vertical);

    //-------------------------------------------------------------------------------------------------
    // Loads, compiles and links a shader program with only VS and FS
    // returns the shader program name, or 0 if no shader program could be created
    GLuint LoadSimpleProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

    //-------------------------------------------------------------------------------------------------
    // Loads, compiles and links a shader program from the files specified in shaderPaths and types specified in shaderTypes
    // returns the shader program name, or 0 if no shader program could be created
    GLuint LoadProgram(std::string shaderPaths[], GLuint shaderTypes[], int size);

    //-------------------------------------------------------------------------------------------------
    // get the time elapsed since program start in seconds
    // NOTE: dimi: time_ms = 1000.0 * ElapsedTime();
    double ElapsedTime();

    //-------------------------------------------------------------------------------------------------
    // mouse buttons
    enum MouseButton { LEFT_MOUSE_BUTTON, MIDDLE_MOUSE_BUTTON, RIGHT_MOUSE_BUTTON };

    // checks if the specified mouse button was pressed since the last call of this function
    // position - if specified, provides the mouse position at the moment the button was pressed
    bool MouseButtonPressed(MouseButton button, glm::ivec2* position = 0);

    //-------------------------------------------------------------------------------------------------
    // checks if the specified mouse button was released since the last call of this function
    // position - if specified, provides the mouse position at the moment the button was released
    bool MouseButtonReleased(MouseButton button, glm::ivec2* position = 0);

    //-------------------------------------------------------------------------------------------------
    // retrives the current mouse position
    glm::ivec2 MousePosition();

    //-------------------------------------------------------------------------------------------------
    // checks if a keyboard key, specified as an ASCII character, was pressed since the last call of this function
    // mousePosition - if specified, provides the mouse position at the moment the key was pressed
    bool KeyPressed(char button, glm::ivec2* mousePosition = 0);

    //-------------------------------------------------------------------------------------------------
    // checks if a keyboard key, specified as an ASCII character, was released since the last call of this function
    // mousePosition - if specified, provides the mouse position at the moment the key was released
    bool KeyReleased(char button, glm::ivec2* mousePosition = 0);

    //-------------------------------------------------------------------------------------------------
    // helper -- do not use directly
    struct Vertex {
        glm::vec3 position;
        glm::vec2 uv;
        glm::vec3 normal;
    };

    // helper -- do not use directly
    enum ObjectFileFlags {
        HAS_UVS = 0x01,
        HAS_NORMALS = 0x02
    };

    // helper -- do not use directly
    bool LoadObjectFile(const std::string& filename, std::vector<Vertex>& vertices, std::vector<int>& indices, unsigned char& flags);
    //-------------------------------------------------------------------------------------------------
    // loads a model from an *.OBJ file, creates the VBO, IBO and VAO for this object with vertex shader attribute locations as specified
    // by posLoc, normLoc and uvLoc
    // returns the VAO name on success, or 0 otherwise; the IBO name and size are written in iboID and iboSize

    // Example: loading an object
    // GLunit  iboID;
    // GLsizei int numIndices;
    // GLuint vaoID = LoadObjectFile("my_model.obj", 0, 1, 2, iboID, numIndices); // VS format: layout(location=0) vec3 position;
    //                                                                            //            layout(location=1) vec3 normal;
    //                                                                            //            layout(location=2) vec2 textureCoordinate;

    // Example: rendering the object loaded
    // glBindVertexArray(vaoID);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
    // glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);

    //GLuint LoadObjectFile(const std::string& filename, int posLoc, int normLoc, int uvLoc, GLuint& iboID, GLsizei& iboSize);

    //-------------------------------------------------------------------------------------------------
	// mesh works
	void GenMeshes(GLsizei n, GLuint* meshes);
	void DeleteMeshes(GLsizei n, GLuint* meshes);

	void DrawMesh(GLuint mesh);
	void DrawMeshInstanced(GLuint mesh, GLsizei numInstances);
	void MultiDrawMesh(GLuint* meshes, GLsizei first, GLsizei count);
	void MultiDrawMeshInstanced(GLuint* meshes, GLsizei first, GLsizei count, GLsizei numInstances);

	// utils
	void CreatePlane(GLuint mesh, float width, float depth, float heightOffset, int wdtiles, int dttiles);
	void CreateDisc(GLuint mesh, float outerRadius, float innerRadius, float startAngle, float endAngle,
		float heightOffset, int slices, int rings);
	void CreateSphere(GLuint mesh, float radius, int slices, int tiles);
	void CreateCylinder(GLuint mesh, float lowerRadius, float upperRadius, float height, int slices,
		int tiles, int sliceSubdivs = 1, bool lowerCap = true, bool upperCap = true);
	void CreateBox(GLuint mesh, float width, float height, float depth, int wdtiles, int httiles, int dttiles);
	void CreateTorus(GLuint mesh, float outerRadius, float innerRadius, int slices, int rings);
	void LoadObject(GLuint mesh, const std::string& filename);

	//-------------------------------------------------------------------------------------------------
    // Image loader suitable when you control the images you're loading and can
    // avoid problematic images and only need the trivial interface
    //
    // JPEG baseline (no JPEG progressive)
    // PNG 8-bit-per-channel only
    //
    // TGA (not sure what subset, if a subset)
    // BMP non-1bpp, non-RLE
    // PSD (composited view only, no extra channels)
    //
    // GIF (*comp always reports as 4-channel)
    // HDR (radiance rgbE format)
    // PIC (Softimage PIC)
    //
    // non-HDR formats support 8-bit samples only (jpeg, png)
    // no delayed line count (jpeg) -- IJG doesn't support either

    // Example usage:
    // GLenum target;	// GL_TEXTURE_1D or GL_TEXTURE_2D
    // GLenum textureID = LoadTexture("my_texture.png", target, true);
    // if (!textureID || target != GL_TEXTURE_2D) {
    //      ERROR.. cannot properly load the texture!!!
    // }
    // glActiveTexture(GL_TEXTURE0 + ...);
    // glBindTexture(target, textureID);

    GLuint LoadTexture(const std::string& filename, GLenum& target, bool autoCreateMipmaps = true);

    //GLuint LoadFont(const std::string& fontTexture, int numSymbols);

	//-------------------------------------------------------------------------------------------------
	// some usefull functions
	const char dont_change = (char)0x80;
	enum color_index{white, lt_gray, gray, dark_gray, black, red, green, blue, yellow, cyan, magenta, def = white};
	enum modifier_id{plain, bf = 0x01, it = 0x02, uline = 0x04, strike = 0x08};

	inline std::string ccolor(int index) {return std::string() + '\a' + (char)index + dont_change;}
	inline std::string cmodifier(int md) {return std::string() + '\a' + dont_change + (char)md;}
	inline std::string ccm(int color, int modifier = plain) {return std::string() + '\a' + (char)color + (char)modifier;}
	inline std::string creset() {return std::string() + '\a' + (char)def + (char)plain;}

	//-------------------------------------------------------------------------------------------------
	// console
	// TODO: dimi: add some functions to change the palette colors
	class Console {
	public:
		// indices of the default palette colors
		struct ColorIndex {
			enum Value {
				White,
				LightGray,
				Gray,
				DarkGray,
				Black,
				Red,
				Green,
				Blue,
				Yellow,
				Cyan,
				Magenta,
				//CustomColor0,
				//CustomColor1,
				//CustomColor2,
				//CustomColor3,
				//CustomColor4,
				//CustomColor5,
				//CustomColor6,
				//CustomColor7,
				//CustomColor8,
				//CustomColor9,
				//PaletteSize,
				Default = White,
			};
		};

		// font modifier
		struct Modifier {
			enum Value {
				Plain	= 0,
				Bold	= 0x01,
				Italic	= 0x02,
				BoldItalic	= 0x03,
				Underline = 0x04,
				StrikeThrough = 0x08
			};
		};

		// ctor
		Console(const glm::vec2& offset, const glm::vec2& size);

		// dtor
		~Console();

		/// Write text into console
		void write(const std::string& text, ColorIndex::Value color = ColorIndex::Default,
			unsigned int modifier = (unsigned int)Modifier::Plain);

		void write(const std::string& text, unsigned int modifier);

		void writeLine(const std::string& text, ColorIndex::Value color = ColorIndex::Default,
			unsigned int modifier = (unsigned int)Modifier::Plain);

		void writeline(const std::string& text, unsigned int modifier);

		template <typename T> Console& operator << (const T& op) {
			std::ostringstream oss;
			oss << op;
			_appendString(oss.str());
			//write(oss.str(), _currentColorIndex, _currentModifier);
			return *this;
		}

		Console& operator<< (std::ostream& (*manipulator) (std::ostream&));

		void resize(int wd, int ht);

		// console offset - [0,1]x[0,1] top-left
		void setConsoleOffset(const glm::vec2& offset);
		glm::vec2 consoleOffset() const;

		// console size - [0,1]x[0,1]
		void setConsoleSize(const glm::vec2& size) ;
		glm::vec2 consoleSize() const;

		void setFontScale(float fontScale);
		float fontScale() const;

		/// Render console
		void render();

		/// visibility
		void setVisible(bool v);
		void toggleVisible();
		bool visible() const;

	private:
		// some usefull constants
		static const int _FONT_MAP_START_ASCII_INDEX;
		static const int _FONT_MAP_END_ASCII_INDEX;
		static const int _MAX_CONSOLE_BUFFER_SIZE;
		static const int _TAB_SIZE;

		// buffer to hold the VBO data
		struct ConsoleCharacter {
			// default ctor
			ConsoleCharacter();

			// parameter ctor
			ConsoleCharacter(char c, ColorIndex::Value v = ColorIndex::Default, Modifier::Value m = Modifier::Plain);

			unsigned char character;
			unsigned char color;
			unsigned char modifier;
		};

		// helper
		void _initialize(const std::string& fontMapFileName, int numCharacters);
		void _invalidateVBO();
		void _updateVBO();
		void _invalidataShaderUniforms();
		void _updateShaderUniforms();
		void _requestRebuild();
		void _rebuild();

		void _appendString(const std::string& str);
		void _appendStringToCharBuffer(const std::string& str);
		void _rebuildCharacterBuffer();

		// --- members ----------------------------------------------------------------------
		std::vector<ConsoleCharacter> _characterBuffer;
		// very simple markup string to hold the current console text..
		// - the only tag in the markup is \a<color-index-byte><modifier-byte>
		std::string _textBuffer;
		// console props -- (some of them) can be set by the user
		glm::vec2	_fontPixelSize; // can be float
		float		_fontScale;
		glm::ivec2	_windowResolution;
		glm::vec2	_consoleRelativeOffset; // top left [0;1]x[0;1]
		glm::vec2	_consoleRelativeSize;		// [0;1]x[0;1]

		// shader props -- calculated, the shader needs (most of) these
		glm::vec2 _offset;
		glm::ivec2 _consoleSize; // symbols x lines
		glm::vec2 _fontSize;
		bool  _needsBlanksAtEol;
		int _consoleNumCharacters;

		// members
		GLuint _vao;
		GLuint _program;

		GLuint _characterVBO;

		GLuint _consoleSizeUniformLocation;
		GLuint _consoleOffsetUniformLocation;
		GLuint _fontSizeUniformLocation;

		GLuint _colorMapUniformLocation;
		GLuint _fontTexture;

		// current color and modifier
		ColorIndex::Value _currentColorIndex;
		Modifier::Value   _currentModifier;

		int linePosition;

		// flags
		bool _visible;
		bool _dirtyBitVBO;
		bool _dirtyBitUniforms;
		bool _needsRebuild;
	}; // Console
} // namespace viscg
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
#endif // VISCG_CG_FRAMEWORK_H