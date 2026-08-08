float distance(const float4& p1, const float4& p2) {
	float dx = p2.x - p1.x;
	float dy = p2.y - p1.y;
	float dz = p2.z - p1.z;

	return sqrt(dx * dx + dy * dy + dz * dz);
}

float4 lerp3(const float4& a, const float4& b, float t) {
	return float4{
		a.x + t * (b.x - a.x),
		a.y + t * (b.y - a.y),
		a.z + t * (b.z - a.z),
		a.w // Сохраняем оригинальное значение w из первой точки
	};
}

float frac(float x) {
	return x - floor(x);
}

float4 normalize(const float4& v) {
	// Считаем длину вектора по формуле Пифагора
	float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

	// Защита от деления на ноль (если вектор нулевой)
	if (length < 0.00001f) {
		return float4{ 0.0f, 0.0f, 0.0f, v.w };
	}

	// Возвращаем нормализованный вектор
	return float4{
		v.x / length,
		v.y / length,
		v.z / length,
		v.w // Поле w оставляем оригинальным
	};
}

float4 cross(const float4& a, const float4& b) {
	return float4{
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x,
		0.0f // Для векторов направления w обычно равен 0
	};
}

namespace Object {

	cmd(Show, 
		texture geometry,
		texture normals,
		int8u quality,
		int pos_x,
		int pos_y,
		int pos_z,
		int glow
		)
	{
		reflect;

		#if EditMode //dynamic limits
			auto r = max(Textures::Texture[(int)in.geometry].size.x, Textures::Texture[(int)in.geometry].size.y);
			auto mipMaps = Textures::Texture[(int)in.geometry].mipMaps;
			cmdParamDesc[cmdCounter - 1].param[2]._min = 0;
			cmdParamDesc[cmdCounter - 1].param[2]._max = max((mipMaps ? (UINT)(_log2(r)) : 0)-2,0);
		#endif

		int denom = (int)pow(2, (float)in.quality);
		float q = intToFloatDenom;
		
		int gX = Textures::Texture[(int)in.geometry].size.x / denom;
		int gY = Textures::Texture[(int)in.geometry].size.y / denom;

		vs::objViewer = {

			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(in.pos_x / q, in.pos_y / q, in.pos_z / q)),
				.gX = gX,
				.gY = gY,
				.glow_p = (float)in.glow,
			},

			.textures = {
				.positions = in.geometry,
				.normals = in.normals
				},

			.samplers = {
				.sam1Filter = filter::linear,
				.sam1AddressU = addr::wrap,
				.sam1AddressV = addr::clamp
			}
		};

		vs::objViewer.set();

		ps::basic = 
		{
			.params = {
				#if EditMode
					.hilight = cmdCounter - 1 == hilightedCmd ? 1.f : 0.f
				#else 
					.hilight = 0.f
				#endif
				}

		};

		ps::basicLow =
		{
			.params = {
				#if EditMode
					.hilight = cmdCounter - 1 == hilightedCmd ? 1.f : 0.f
				#else 
					.hilight = 0.f
				#endif
				}

		};

		if (in.glow == 1)
		{
			ps::basic.set();
		}
		else
		{
			ps::basicLow.set();
		}
		

		//Drawer::NullDrawer({(int)gX*(int)gY,1});
		if (in.glow == 0)
		{
			Drawer::NullDrawer({ 1, (int)gX * (int)gY/ 10394 });
		}
		else
		{
			Drawer::NullDrawer({ 1, (int)gX * (int)gY });
		}

		
	}

	

	enum class pMode { point,glow };
	enum class triMode { on,off };
	
	void psModeSet(pMode mode)
	{
		switch (mode)
		{
			case pMode::point:
			{
				ps::basic = { .params = {.hilight = 0.f } };
				ps::basic.set();
				break;
			}
			case pMode::glow:
			{
				ps::basicLow = { .params = {.hilight = 0.f } };
				ps::basicLow.set();
				break;
			}
		}
	}

	void PillarsHand(int count, int skipper, pMode mode)
	{
		int gX = sqrt(count / skipper);
		int gY = sqrt(count / skipper);

		psModeSet(mode);

		vs::pillarsHand = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)mode,
				.skipper = skipper,
			},
		};

		vs::pillarsHand.set();

		Drawer::NullDrawer({ 1, (int)gX * (int)gY });
	}

	cmd(InsideNebula, int count, int skipper, pMode mode, int r, int g, int b)
	{
		reflect;

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		vs::insideNebula = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)in.mode,
				.skipper = in.skipper,
				.base_color=float4(in.r/100.,in.g/100.,in.b/100.,1)
			},
		};

		vs::insideNebula.set();

		Drawer::NullDrawer({1,(int)gX*(int)gY});

		
	}

	cmd(Blob, int count, int skipper, pMode mode, int r, int g, int b)
	{
		reflect;

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		vs::blob = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)in.mode,
				.skipper = in.skipper,
				.base_color = float4(in.r / 100.,in.g / 100.,in.b / 100.,1)
			},
		};

		vs::blob.set();

		Drawer::NullDrawer({ 1,(int)gX * (int)gY });

	
	}

	cmd(Pearl, int count, int skipper, pMode mode, int r, int g, int b)
	{
		reflect;

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		vs::pearl = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)in.mode,
				.skipper = in.skipper,
				.base_color = float4(in.r / 100.,in.g / 100.,in.b / 100.,1)
			},
		};

		vs::pearl.set();

		Drawer::NullDrawer({ 1,(int)gX * (int)gY });

	
	}

	cmd(LeoStar, int count, int skipper, pMode mode, int r, int g, int b)
	{
		reflect;

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		vs::leo = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)in.mode,
				.skipper = in.skipper,
				.base_color = float4(in.r / 100.,in.g / 100.,in.b / 100.,1)
			},
		};

		vs::leo.set();

		Drawer::NullDrawer({ 1,(int)gX * (int)gY });

	
	}

	cmd(CapStar, int count, int skipper, pMode mode, int r, int g, int b)
	{
		reflect;

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		vs::capriStar = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)in.mode,
				.skipper = in.skipper,
				.base_color = float4(in.r / 100.,in.g / 100.,in.b / 100.,1)
			},
		};

		vs::capriStar.set();

		Drawer::NullDrawer({ 1,(int)gX * (int)gY });

		
	}

	

	cmd(Tau, int count, int skipper, pMode mode, int r, int g, int b)
	{
		reflect;

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		vs::Tau = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)in.mode,
				.skipper = in.skipper,
				.base_color = float4(in.r / 100.,in.g / 100.,in.b / 100.,1),
				//.mesh = {
					//#include "girl.h"
					//#include "girl_rand.h"
					//#include "girl_mini.h"
				//}
			},
		};
		
		vs::Tau.set();

		Drawer::NullDrawer({ 1,(int)gX * (int)gY });

		
	}

#if EditMode

	dx11::ConstBuf::sbObject HeroMesh;
	dx11::ConstBuf::sbObject BossMesh;
	dx11::ConstBuf::sbObject* MeshPtr = NULL;
	XMMATRIX heroOnRails;
	XMMATRIX heroWorld;

	void ShowMesh(dx11::ConstBuf::sbObject* obj, int count, int skipper, pMode mode, int r, int g, int b, triMode tMode, int xPos, int yPos, int zPos, int brightness, int tickness,int zoom, int onLineOfs, int jumpCharge)
	{

		int gX = sqrt(count / skipper);
		int gY = sqrt(count / skipper);

		psModeSet(mode);
		float zm = zoom / 100. + 1;

		vs::girl = {
			.params =
			{
				.model = heroWorld,
				.gX = gX,
				.gY = gY,
				.mode = (int)mode,
				.skipper = skipper,
				.base_color = float4(r / 100.,g / 100.,b / 100.,1),
				.modelPos = float4(xPos/10000.,yPos / 10000.,zPos / 10000.,0),
				.triCount = float4(obj->triangleCount,0,0,0),
				.brightness = float4(brightness,0,0,0),
				.tickness = float4(tickness,0,0,0),
				.zoom = float4(zm,zm,zm,1),
				.onLineOfs = (float)onLineOfs/1000.f,
				.jumpCharge = (float)jumpCharge / 100.f,
			},
		};

		if (tMode == triMode::on)
		{
			vs::girl.params.mode = 2;
		}

		vs::girl.set();

		obj->BindSB(0);
		obj->BindSB(1);


		if (tMode == triMode::on)
		{
			//dx11::Shaders::resetShader(dx11::Shaders::basic);
			//dx11::Shaders::resetShader(dx11::Shaders::basic);
			Drawer::NullDrawerTri({ count, 1 });
		}
		else
		{
			Drawer::NullDrawer({ 1,(int)gX * (int)gY });
		}

		
	}

	cmd(Mesh, int quality, int xPos, int yPos, int zPos, int brightness, int tickness, switcher stencil,int zoom, int onLineOfs, int jumpCharge)
	{
		reflect;

		int count = 500000;

		DepthBuf::Mode({ depthmode::on });
		BlendMode::Set({
			.mode = blendmode::off,
			.op = blendop::add
			});

		Culling::Set({ cullmode::off });
		if (in.stencil == switcher::on)
		{
			ShowMesh(MeshPtr, (int)MeshPtr->triangleCount,1,pMode::point,0,0,0, triMode::on, in.xPos, in.yPos, in.zPos,in.brightness,in.tickness,in.zoom,in.onLineOfs, in.jumpCharge);
		}

		Culling::Set({ cullmode::off });
		DepthBuf::Mode({ depthmode::readonly });
		BlendMode::Set({
			.mode = blendmode::on,
			.op = blendop::add
			});

		ShowMesh(MeshPtr, count, 1, pMode::point, 100, 252, 1400, triMode::off, in.xPos, in.yPos, in.zPos, in.brightness, in.tickness,in.zoom, in.onLineOfs, in.jumpCharge);
	}

#endif

	cmd(ScorpBall, int count, int skipper, pMode mode, int r, int g, int b)
	{
		reflect;

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		vs::scorpBall = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)in.mode,
				.skipper = in.skipper,
				.base_color = float4(in.r / 100.,in.g / 100.,in.b / 100.,1)
			},
		};

		vs::scorpBall.set();

		Drawer::NullDrawer({ 1,(int)gX * (int)gY });

		
	}

	cmd(Nebula2, int count, int skipper, pMode mode, int r, int g, int b)
	{
		reflect;

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		vs::Nebula2 = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)in.mode,
				.skipper = in.skipper,
				.base_color = float4(in.r / 100.,in.g / 100.,in.b / 100.,1)
			},
		};

		vs::Nebula2.set();

		Drawer::NullDrawer({ 1,(int)gX * (int)gY });

		
	}

	cmd(vrg, int count, int skipper, pMode mode, int r, int g, int b)
	{
		reflect;

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		XMMATRIX invViewMatrix = XMMatrixInverse(nullptr, XMMatrixTranspose(ConstBuf::camera.view[0]));
		// Позиция камеры находится в 4-й строке инвертированной матрицы (вектор смещения)
		XMVECTOR cameraPos = invViewMatrix.r[3];
		// Если нужно получить отдельные float:
		XMFLOAT3 eye;
		XMStoreFloat3(&eye, cameraPos);

		XMVECTOR cameraLookAtVec = XMVector3Normalize(invViewMatrix.r[2]);

		// Сохраняем в структуру XMFLOAT3 для передачи в Shader Constants / Constant Buffer
		XMFLOAT3 cameraForward;
		XMStoreFloat3(&cameraForward, cameraLookAtVec);

		// 1. Извлекаем и нормализуем вектор Right (1-я строка инвертированной матрицы)
		XMVECTOR cameraRightVec = XMVector3Normalize(invViewMatrix.r[0]);

		// 2. Извлекаем и нормализуем вектор Up (2-я строка инвертированной матрицы)
		XMVECTOR cameraUpVec = XMVector3Normalize(invViewMatrix.r[1]);

		// Сохраняем в структуры XMFLOAT3 для передачи в ваш Constant Buffer
		XMFLOAT3 cameraRight;
		XMStoreFloat3(&cameraRight, cameraRightVec);

		XMFLOAT3 cameraUp;
		XMStoreFloat3(&cameraUp, cameraUpVec);

		vs::Virgo = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)in.mode,
				.skipper = in.skipper,
				.base_color = float4(in.r / 100.,in.g / 100.,in.b / 100.,1),
				.eye = float4(eye.x,eye.y,eye.z,0),
				.forward = float4(cameraForward.x,cameraForward.y,cameraForward.z,0),
				.up = float4(cameraUp.x,cameraUp.y,cameraUp.z,0),
				.right = float4(cameraRight.x,cameraRight.y,cameraRight.z,0),
			},
		};

		vs::Virgo.set();

		Drawer::NullDrawer({ 1,(int)gX * (int)gY });

		
	}

	const int smoothPointMAX = 1000;

	struct starline {
		float4 basePoint[100];
		float4 point[smoothPointMAX];
		float4 upVector[smoothPointMAX];
		int basePointCount = 0;
		int pointCount = 0;
	};

	struct {
		starline line[100];
		int lineCount = 0;
	} starLineList;

	int currentLine = -1;
	int currentPoint = 0;
	
	cmd(SetLineCount, int lineCount)
	{
		reflect;
		starLineList.lineCount = in.lineCount;
	}

	const int denom = 1;

	cmd(SetPointPosInLine, int line, int point, int x,int y, int z, int a)
	{
		reflect;
		starLineList.line[in.line].basePoint[in.point] = float4(in.x/ (float)denom,in.y/ (float)denom,in.z/ (float)denom,in.a);
	}

	cmd(AddPointToLine, int x, int y, int z, int a=0)
	{
		reflect;
		starLineList.line[currentLine].basePoint[currentPoint++] = float4(in.x / (float)denom, in.y / (float)denom, in.z / (float)denom, in.a);
		starLineList.line[currentLine].basePointCount = currentPoint;
	}

	void AddPoint( float4 p)
	{
		starLineList.line[currentLine].basePoint[currentPoint++] = p;
		starLineList.line[currentLine].basePointCount = currentPoint;
	}

	



	void smoothStarline(starline& line) {
		line.pointCount = 0; // Сбрасываем старый результат сглаживания

		float totalLength = 0;
		for (int i = 0; i < line.basePointCount-1; i++)
		{
			totalLength += distance(line.basePoint[i], line.basePoint[i + 1]);
		}

		int stepsPerSegment = totalLength/25.;
		if (stepsPerSegment < 2) stepsPerSegment = 2;

		// Если исходных точек недостаточно для сглаживания или шаг некорректен
		if (line.basePointCount < 2 || stepsPerSegment <= 0) {
			// Просто копируем исходные точки в результирующий массив
			int limit = (line.basePointCount > smoothPointMAX) ? smoothPointMAX : line.basePointCount;
			for (int i = 0; i < limit; ++i) {
				line.point[i] = line.basePoint[i];
			}
			line.pointCount = limit;
			return;
		}

		// Проходим по сегментам между исходными точками basePoint
		for (int i = 0; i < line.basePointCount - 1; ++i) {
			// Формируем 4 опорные точки для Кэтмулла-Рома (с виртуальным продлением на краях)
			float4 p0 = (i == 0) ? line.basePoint[i] : line.basePoint[i - 1];
			float4 p1 = line.basePoint[i];
			float4 p2 = line.basePoint[i + 1];
			float4 p3 = (i == line.basePointCount - 2) ? line.basePoint[i + 1] : line.basePoint[i + 2];

			// Генерируем промежуточные точки внутри текущего сегмента
			for (int step = 0; step < stepsPerSegment; ++step) {
				// Защита от переполнения жестко ограниченного массива point[100]
				if (line.pointCount >= smoothPointMAX) {
					return;
				}

				float t = (float)step / (float)stepsPerSegment;
				line.point[line.pointCount] = catmullRom(p0, p1, p2, p3, t);
				line.pointCount++;
			}
		}

		// Добавляем финальную опорную точку, чтобы линия завершилась корректно
		if (line.pointCount < smoothPointMAX) {
			line.point[line.pointCount] = line.basePoint[line.basePointCount - 1];
			line.pointCount++;
		}
	}

	

	void Starline(starline& line, int stepsPerSegment) {
		line.pointCount = 0; // Сбрасываем старый результат сглаживания

		// Проходим по сегментам между исходными точками basePoint
		for (int i = 0; i < line.basePointCount - 1; ++i) {

			// Генерируем промежуточные точки внутри текущего сегмента
			for (int step = 0; step < stepsPerSegment; ++step) {
				// Защита от переполнения жестко ограниченного массива point[100]
				if (line.pointCount >= smoothPointMAX) {
					return;
				}

				float t = (float)step / (float)stepsPerSegment;
				line.point[line.pointCount] = lerp3(line.basePoint[i],line.basePoint[i+1],t);
				line.pointCount++;
			}
		}

		// Добавляем финальную опорную точку, чтобы линия завершилась корректно
		if (line.pointCount < smoothPointMAX) {
			line.point[line.pointCount] = line.basePoint[line.basePointCount - 1];
			line.pointCount++;
		}
	}


	// Функция плавной интерполяции (Smoothstep / Fade)
	inline float perlin_fade(float t) {
		return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
	}

	// Линейная интерполяция
	inline float perlin_lerp(float t, float a, float b) {
		return a + t * (b - a);
	}

	// Вычисление скалярного произведения с градиентным вектором
	inline float perlin_grad(int hash, float x, float y, float z) {
		int h = hash & 15;
		float u = h < 8 ? x : y;
		float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
		return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	}

	// Вспомогательная функция для получения одного скалярного значения шума
	float GetSinglePerlinNoise3D(float x, float y, float z) {
		// Таблица перестановок Перлина (повторена дважды, чтобы избежать выхода за границы при +1)
		static const int p[512] = {
			151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
			190,6,148,247,120,234,75,0,26,56,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,
			125,136,171,168,68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,
			105,92,41,55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,
			135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,
			82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,
			153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,
			251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,
			157,184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,
			66,215,61,156,180,
			// Повторение массива
			151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
			190,6,148,247,120,234,75,0,26,56,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,
			125,136,171,168,68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,
			105,92,41,55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,
			135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,
			82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,
			153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,
			251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,
			157,184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,
			66,215,61,156,180
		};

		int X = static_cast<int>(std::floor(x)) & 255;
		int Y = static_cast<int>(std::floor(y)) & 255;
		int Z = static_cast<int>(std::floor(z)) & 255;

		x -= std::floor(x);
		y -= std::floor(y);
		z -= std::floor(z);

		float u = perlin_fade(x);
		float v = perlin_fade(y);
		float w = perlin_fade(z);

		int A = p[X] + Y;
		int AA = p[A] + Z;
		int AB = p[A + 1] + Z;
		int B = p[X + 1] + Y;
		int BA = p[B] + Z;
		int BB = p[B + 1] + Z;

		return perlin_lerp(w, perlin_lerp(v, perlin_lerp(u, perlin_grad(p[AA], x, y, z),
			perlin_grad(p[BA], x - 1, y, z)),
			perlin_lerp(u, perlin_grad(p[AB], x, y - 1, z),
				perlin_grad(p[BB], x - 1, y - 1, z))),
			perlin_lerp(v, perlin_lerp(u, perlin_grad(p[AA + 1], x, y, z - 1),
				perlin_grad(p[BA + 1], x - 1, y, z - 1)),
				perlin_lerp(u, perlin_grad(p[AB + 1], x, y - 1, z - 1),
					perlin_grad(p[BB + 1], x - 1, y - 1, z - 1))));
	}

	// Целевая функция, принимающая три аргумента и возвращающая XMVECTOR
	XMVECTOR GetPerlinNoiseVector3(float x, float y, float z) {
		// Смещаем координаты для каждого канала, чтобы значения X, Y и Z не дублировали друг друга
		float nx = GetSinglePerlinNoise3D(x, y, z);
		float ny = GetSinglePerlinNoise3D(x + 31.415f, y + 58.271f, z + 93.123f);
		float nz = GetSinglePerlinNoise3D(x + 115.53f, y + 213.91f, z + 351.67f);

		// Возвращаем упакованный в SIMD-регистр вектор (компонента W = 0.0f)
		return XMVectorSet(nx, ny, nz, 0.0f);
	}

	void NewLine()
	{
		currentLine++;
		currentPoint = 0;
		starLineList.lineCount = currentLine+1;
	}

	float4 gemini[] = {
		// Pollux Line (Right Branch)
		{  0.95f,  0.72f,  0.0f,  0.0f }, // Pollux
		{  0.58f,  0.41f,  0.0f,  1.0f }, // Wasat
		{  0.21f,  0.12f,  0.0f,  2.0f }, // Mebsuta
		{ -0.25f, -0.28f,  0.0f,  3.0f }, // Mekbuda
		{ -0.68f, -0.65f,  0.0f,  4.0f }, // Alhena
		{ -0.92f, -0.85f,  0.0f,  5.0f }, // Alzirr

		// Castor Line (Left Branch)
		{  0.88f,  0.91f,  0.0f,  6.0f }, // Castor
		{  0.45f,  0.62f,  0.0f,  7.0f }, // Kappa Gem
		{  0.12f,  0.35f,  0.0f,  8.0f }, // Upsilon Gem
		{ -0.18f,  0.08f,  0.0f,  9.0f }, // Propus
		{ -0.52f, -0.22f,  0.0f, 10.0f }, // Tejat Posterior
		{ -0.75f, -0.45f,  0.0f, 11.0f }, // Tejat Prior

		// Connecting stars
		{  0.32f,  0.18f,  0.0f, 12.0f }, // Lambda Gem
		{  0.05f,  0.55f,  0.0f, 13.0f }, // Tau Gem
		{ -0.15f,  0.42f,  0.0f, 14.0f }, // Theta Gem
		{ -0.42f,  0.22f,  0.0f, 15.0f }, // Nu Gem
		{ -0.85f, -0.12f,  0.0f, 16.0f }  // 1 Gem
	};

	void genSegment(float4 start, float4 end)
	{
		NewLine();
		int seg = distance(start,end)*20.;
		if (seg == 0) seg += 2;
		for (int k = 0; k <= seg; k++)
		{
			float4 p = lerp3(start, end, k / (float)seg);

			float rs = .02 * sin((k / (float)seg) * PI);
			p.x += getRandFloat() * rs;
			p.y += getRandFloat() * rs;
			p.z += getRandFloat() * rs;

			// Масштабируем координаты точек для игрового мира
			float scale = 600;
			p.x *= scale;
			p.y *= scale;
			p.z *= scale;

			AddPoint(p);
		}
	}

	void initPatches(float pathTime)
	{
		// init maze
		currentLine = -1;
		// 
		//-----------------------------------------
		//-----------start user space--------------
		
		int starsCount = sizeof(gemini) / sizeof(float4);
		
		srand(100);

		/*for (int i = 0; i < starsCount; i++)
		{
			gemini[i].z = getRandFloat();
		}
		
		genSegment(gemini[0], gemini[1]);
		genSegment(gemini[1], gemini[2]);
		genSegment(gemini[2], gemini[3]);
		genSegment(gemini[3], gemini[4]);
		genSegment(gemini[4], gemini[5]);
		genSegment(gemini[6], gemini[7]);
		genSegment(gemini[7], gemini[8]);
		genSegment(gemini[8], gemini[8]);
		genSegment(gemini[9], gemini[10]);
		genSegment(gemini[10], gemini[11]);
		genSegment(gemini[0], gemini[6]);
		genSegment(gemini[7], gemini[12]);
		genSegment(gemini[12], gemini[13]);
		genSegment(gemini[13], gemini[14]);
		genSegment(gemini[14], gemini[15]);
		genSegment(gemini[10], gemini[16]);
		*/
		

		//====================
		// 1 -> 14(1)
		//===================
		NewLine();
		AddPointToLine({ 0,14,0 });
		AddPointToLine({ 10,15,-7 });
		AddPointToLine({ 24,6,-7 });
		AddPointToLine({ 43,-6,-6 });
		AddPointToLine({ 60,-50, -20 });


		//====================
		// 1 -> 14(2)
		//===================
		NewLine();
		AddPointToLine({ 0, 14,   0 });
		// первый виток
		AddPointToLine({ 4, 19,  -2 });
		AddPointToLine({ 11, 18,  -6 });
		AddPointToLine({ 14, 18,  -8 });
		AddPointToLine({ 16, 14,  -4 });
		AddPointToLine({ 10,  7,  -3 });
		AddPointToLine({ 7,  6,  -5 });
		AddPointToLine({ 8,  8,  -9 });
		// второй виток
		AddPointToLine({ 12, 11, -11 });
		AddPointToLine({ 17, 13,  -9 });
		AddPointToLine({ 21, 10,  -5 });
		AddPointToLine({ 22,  6,  -2 });
		AddPointToLine({ 20,  2,   0 });
		AddPointToLine({ 16,  1,  -3 });
		AddPointToLine({ 15,  5,  -7 });

		NewLine();
		// третий виток
		AddPointToLine({ 20,  8, -10 });
		AddPointToLine({ 23, 10,  -7 });
		AddPointToLine({ 29,  7,  -3 });
		AddPointToLine({ 30,  3,   1 });
		AddPointToLine({ 28, -2,   1 });
		AddPointToLine({ 24, -3,  -3 });
		AddPointToLine({ 23,  0,  -7 });
		// четвёртый виток
		AddPointToLine({ 28,  4, -10 });
		AddPointToLine({ 33,  6,  -7 });
		AddPointToLine({ 38,  3,  -3 });
		AddPointToLine({ 39, -2,   0 });
		AddPointToLine({ 37, -6,  -1 });
		AddPointToLine({ 41, -7,  -5 });
		AddPointToLine({ 46, -5,  -17 });

		NewLine();
		// выход
		AddPointToLine({ 38, -4, -13 });
		AddPointToLine({ 44, -2, -12 });
		AddPointToLine({ 49, -7, -13 });
		AddPointToLine({ 53,-16, -15 });
		AddPointToLine({ 55,-27, -17 });
		AddPointToLine({ 57,-39, -19 });
		AddPointToLine({ 60,-50, -20 });
		//
		// 
		// 
		////// 1 -> 14(3)
		//NewLine();
		//AddPointToLine({ 0,14,0 });
		//AddPointToLine({ 15,-32,-16 });
		//AddPointToLine({ 47,-17,19 });
		//AddPointToLine({ 74,3,3 });
		//AddPointToLine({ 93,1,15 });
		//AddPointToLine({ 72,-27,14 });
		//
		//NewLine();
		//AddPointToLine({ 29,-5,21 });
		//AddPointToLine({ 86,-21,25 });
		//AddPointToLine({ 68,10,14 });
		//AddPointToLine({ 45,8,13 });
		//AddPointToLine({ 82,-13,-14 });
		//AddPointToLine({ 76,-49,-4 });
		//
		//NewLine();
		//AddPointToLine({ 98,8,-11 });
		//AddPointToLine({ 110,-39,9 });
		//AddPointToLine({ 77,-15,18 });
		//AddPointToLine({ 61,-16,15 });
		//AddPointToLine({ 64,-26,-3 });
		//AddPointToLine({ 57,-51,27 });
		//
		//NewLine();
		//AddPointToLine({ 74,-65,49 });
		//AddPointToLine({ 54,-63,35 });
		//AddPointToLine({ 32,-37,3 });
		//AddPointToLine({ 66,-18,5 });
		//AddPointToLine({ 109,-26,-5 });
		//AddPointToLine({ 61,-18,-3 });
		//AddPointToLine({ 71,-33,206 });
		//
		//NewLine();
		//AddPointToLine({ 54,-10,86 });
		//AddPointToLine({ 110,6,48 });
		//AddPointToLine({ 114,-17,48 });
		//AddPointToLine({ 72,-18,48 });
		//AddPointToLine({ 92,-18,5 });
		//
		//
		//NewLine();
		//AddPointToLine({ 46,-25,-24 });
		//AddPointToLine({ 57,-28,-5 });
		//AddPointToLine({ 71,-26,-32 });
		//AddPointToLine({ 60,-50,-20 });
		


		//====================		   
		// 14 -> 9					   
		//====================		   
		//NewLine();					   
		//AddPointToLine({ 60,-50,-20 });
		//AddPointToLine({ 61,-65,-33 });
		//AddPointToLine({ 24,-41,-2 });
		//
		//AddPointToLine({ 41,-68,-2 });
		//AddPointToLine({ 48,-5,6 });
		//AddPointToLine({ 81,-11,-2 });

		
		//AddPointToLine({ 52,-26,-28 });
		//AddPointToLine({ 70,-27,-49 });
		//AddPointToLine({ 82,-33,-41 });
		//							   
		//AddPointToLine({ 77,-6,-35 }); 
		//AddPointToLine({ 98,-26,-57 });
		//AddPointToLine({ 61,-40,-39 }); 
		//
		
		//AddPointToLine({ 28,2,-39 }); 
		//AddPointToLine({ 56,4,-72 }); 
		//AddPointToLine({ 72,-23,-59 });
		//
		//AddPointToLine({ 53,-23,-51 });
		//AddPointToLine({ 107,-88,-53 });
		//AddPointToLine({ 100,-55,-55 });
		//							   
		//AddPointToLine({ 84,1,-16 }); 
		//AddPointToLine({ 78,21,-40 }); 
		//AddPointToLine({ 69,5,-56 }); 
		//
		//AddPointToLine({ 89,16,-48 }); 
		//AddPointToLine({ 119,51,-53 }); 
		//AddPointToLine({ 69,15,-49 });
		//
	
		//AddPointToLine({ 23,-24,-50 });
		//AddPointToLine({ 32,95,-64 }); 
		//AddPointToLine({ 24,40,-32 }); 
		//							   
		//AddPointToLine({ 36,34,60 });  
		//AddPointToLine({ 28,1,-5 });  
		//AddPointToLine({ 44,64,-26 });
		//
		//AddPointToLine({ 47,44,-33 }); 
		//AddPointToLine({ 33,41,-6 }); 
		//AddPointToLine({ 56,48,-54 }); 
		//
		
		//AddPointToLine({ 31,22,-57 }); 
		//AddPointToLine({ 26,5,-11 });	   
		//AddPointToLine({ 42,47,-4 });  
		//							   
		//AddPointToLine({ 70,24,-58 }); 
		//AddPointToLine({ 35,40,-65 }); 

		//====================
		// 14 -> 12
		//====================
		//NewLine();
		//AddPointToLine({ 60,-50,-20 });
		//AddPointToLine({ 60,-50,-20 });
		//AddPointToLine({ 60,-50,-20 });
		//
		//AddPointToLine({ 75,0,-16 });
		//AddPointToLine({ 75,0,-16 });
		//AddPointToLine({ 75,0,-16 });
		//
		//AddPointToLine({ 90,0,-12 });
		//AddPointToLine({ 90,0,-12 });
		//AddPointToLine({ 90,0,-12 });
		//
		//AddPointToLine({ 105,-100,-8 });
		//AddPointToLine({ 105,-100,-8 });
		//AddPointToLine({ 105,-100,-8 });
		//
		////====================
		//// 14 -> 8
		////====================
		//NewLine();
		//AddPointToLine({ 60,-50,-20 });
		//AddPointToLine({ 62,-39,-43 });
		//AddPointToLine({ 89,-46,-59 });
		//
		//AddPointToLine({ 60,-79,-48 });
		//AddPointToLine({ 109,-29,-38 });
		//AddPointToLine({ 134,-81,-51 });
		//
		//AddPointToLine({ 148,-71,-28 });
		//AddPointToLine({ 153,-81,-63 });
		//AddPointToLine({ 134,-35,-66 });
		//
		//AddPointToLine({ 78,-72,-33 });
		//AddPointToLine({ 122,-36,12 });
		//AddPointToLine({ 150,-44,-45 });
		//
		//AddPointToLine({ 111,-71,-2 });
		//AddPointToLine({ 95,-15,-57 });
		//AddPointToLine({ 127,-6,-53 });
		//AddPointToLine({ 87,-26,-61 });
		//AddPointToLine({ 137,-6,-94 });
		//AddPointToLine({ 113,-34,-84 });
		//
		//AddPointToLine({ 79,33,-68 });
		//AddPointToLine({ 62,11,-48 });
		//AddPointToLine({ 48,17,-57 });
		//
		//AddPointToLine({ 54,-18,-83 });
		//AddPointToLine({ -21,-6,-120 });
		//AddPointToLine({ 57,-20,-90 });
		//
		//AddPointToLine({ 86,-20,-84 });
		//AddPointToLine({ 119,-10,-74 });
		//AddPointToLine({ 48,-36,-20 });
		//
		//AddPointToLine({ 34,-3,-68 });
		//AddPointToLine({ 55,2,-111 });
		//AddPointToLine({ 74,54,-71 });
		//
		//AddPointToLine({ 107,70,-79 });
		//AddPointToLine({ 51,54,-47 });
		//AddPointToLine({ 71,69,-58 });
		//
		//AddPointToLine({ 147,63,-71 });
		//AddPointToLine({ 103,54,-105 });
		//AddPointToLine({ 116,44,-125 });
		//
		//AddPointToLine({ 108,-13,-110 });
		//AddPointToLine({ 126,-42,-75 });
		//AddPointToLine({ 90,-25,-110 });
		//
		////====================
		//// 8 -> 10
		////====================
		//NewLine();
		//AddPointToLine({ 90,-25,-110 });
		//AddPointToLine({ 90,0,-143 });
		//AddPointToLine({ 90,0,-176 });
		//AddPointToLine({ 90,0,-210 });
		//
		////====================
		//// 10 -> 7
		////====================
		//NewLine();
		//AddPointToLine({ 90,0,-210 });
		//AddPointToLine({ 120,0,-233 });
		//AddPointToLine({ 150,0,-256 });
		//AddPointToLine({ 180,0,-280 });
		//
		////====================
		//// 8 -> 13
		////====================
		//NewLine();
		//AddPointToLine({ 90,-25,-110 });
		//AddPointToLine({ 117,0,-130 });
		//AddPointToLine({ 143,0,-150 });
		//AddPointToLine({ 170,0,-170 });
		//
		////====================
		//// 13 -> 3
		////====================
		//NewLine();
		//AddPointToLine({ 170,0,-170 });
		//AddPointToLine({ 200,0,-197 });
		//AddPointToLine({ 230,0,-223 });
		//AddPointToLine({ 260,0,-250 });
		//
		////====================
		//// 2 -> 17
		////====================
		//NewLine();
		//AddPointToLine({ 40,0,70 });
		//AddPointToLine({ 93,0,58 });
		//AddPointToLine({ 147,0,46 });
		//AddPointToLine({ 200,0,35 });
		//
		////====================
		//// 12 -> 17
		////====================
		//NewLine();
		//AddPointToLine({ 105,-100,-8 });
		//AddPointToLine({ 137,0,6 });
		//AddPointToLine({ 169,0,21 });
		//AddPointToLine({ 200,0,35 });
		//
		////====================
		//// 17 -> 11
		////====================
		//NewLine();
		//AddPointToLine({ 200,0,35 });
		//AddPointToLine({ 230,0,57 });
		//AddPointToLine({ 260,0,78 });
		//AddPointToLine({ 290,0,100 });
		//
		////====================
		//// 17 -> 5
		////====================
		//NewLine();
		//AddPointToLine({ 200,0,35 });
		//AddPointToLine({ 217,0,10 });
		//AddPointToLine({ 233,0,-15 });
		//AddPointToLine({ 250,0,-40 });
		//
		////====================
		//// 5 -> 15
		////====================
		//NewLine();
		//AddPointToLine({ 250,0,-40 });
		//AddPointToLine({ 270,0,-83 });
		//AddPointToLine({ 290,0,-126 });
		//AddPointToLine({ 310,0,-170 });
		//
		////====================
		//// 5 -> 4
		////====================
		//NewLine();
		//AddPointToLine({ 250,0,-40 });
		//AddPointToLine({ 280,0,-58 });
		//AddPointToLine({ 310,0,-77 });
		//AddPointToLine({ 340,0,-95 });
		//
		////====================
		//// 4 -> 6
		////====================
		//NewLine();
		//AddPointToLine({ 340,0,-95 });
		//AddPointToLine({ 353,0,-95 });
		//AddPointToLine({ 367,0,-95 });
		//AddPointToLine({ 380,0,-95 });
		//
		////====================
		//// 6 -> 16
		////====================
		//NewLine();
		//AddPointToLine({ 380,0,-95 });
		//AddPointToLine({ 397,0,-87 });
		//AddPointToLine({ 413,0,-78 });
		//AddPointToLine({ 430,0,-70 });


		/*for (int i = 0; i < starsCount; i++)
		{
			// Начинаем с i + 1, чтобы не проверять i==j и не дублировать пары (j,i)
			for (int j = i + 1; j < starsCount; j++)
			{
				float4 start = gemini[i];
				float4 end = gemini[j];

				// Проверка дистанции в исходном диапазоне -1...1
				if (distance(start, end) < .75)
				{
					NewLine();
					int seg = 10;
					for (int k = 0; k <= seg; k++)
					{
						float4 p = lerp3(start, end, k / (float)seg);

						float rs = .02*sin((k / (float)seg)*PI);
						p.x += getRandFloat()*rs;
						p.y += getRandFloat()*rs;
						p.z += getRandFloat()*rs;

						// Масштабируем координаты точек для игрового мира
						float scale = 600;
						p.x *= scale;
						p.y *= scale;
						p.z *= scale;

						AddPoint(p);
					}
				}
			}
		}*/

			
		//------------end user space---------------
		//-----------------------------------------

		for (int j = 0; j < starLineList.lineCount; j++)
		{
			smoothStarline(starLineList.line[j]);
			//Starline(starLineList.line[j], 3*12. / starLineList.line[j].basePointCount);
		}

		/*pathTime /= 100.;

		for (int j = 0; j < 3; j++)
		{
			ln = j;
			SetPointCountInLine({ ln,7 });
			pt = 0;
			for (int i = 0; i < 7; i++)
			{
				float4 pos;
				float amp = 10 * 10000 * ((i+1) / (j + .3) + 4);
				//pos.x = amp * sin(i * 13 + pathTime);
				//pos.y = amp * cos(i * 14 + pathTime);
				//pos.z = amp * sin(i * 15 + pathTime);

				auto v = GetPerlinNoiseVector3(ln+i * .13 + pathTime, ln+i * .25 + pathTime, ln+i * .37 + pathTime) * amp;
				pos.x = XMVectorGetX(v);
				pos.y = XMVectorGetY(v);
				pos.z = XMVectorGetZ(v);

				SetPointPosInLine({ ln,pt++, (int)pos.x,(int)pos.y,(int)pos.z,0,10000 });

			}

			smoothStarline(starLineList.line[ln], 7);
		}


		//
		*/
	}

	float4 hero_pos;

	cmd(Maze, int count, int skipper, pMode mode, int r, int g, int b)
	{
		reflect;

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		for (int i = 0; i < starLineList.lineCount; i++)
		{

			vs::maze = {
				.params = {
					.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
					.gX = gX,
					.gY = gY,
					.mode = (int)in.mode,
					.skipper = in.skipper,
					.heroPosition = hero_pos,
					.base_color = float4(in.r / 100.,in.g / 100.,in.b / 100.,1),
				},
			};

			vs::maze.params.particlesCount = in.count;
			vs::maze.params.basePointsCount = starLineList.line[i].pointCount;

			for (int j = 0; j < starLineList.line[i].pointCount; j++)
			{
				vs::maze.params.basePoint[j] = starLineList.line[i].point[j];
			}

			vs::maze.set();

			Drawer::NullDrawer({ 1,in.count / in.skipper });
		}


	}

	cmd(Rocks, int count, int skipper, pMode mode, int r, int g, int b)
	{
		reflect;

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		vs::rocks = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)in.mode,
				.skipper = in.skipper,
				.base_color = float4(in.r / 100.,in.g / 100.,in.b / 100.,1)
			},
		};

		vs::rocks.set();

		Drawer::NullDrawer({ 1,(int)gX * (int)gY });

		
	}

	cmd(Transporter, int count, int skipper, pMode mode, int r, int g, int b)
	{
		reflect;

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		vs::transporter = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)in.mode,
				.skipper = in.skipper,
				.base_color = float4(in.r / 100.,in.g / 100.,in.b / 100.,1)
			},
		};

		vs::transporter.set();

		Drawer::NullDrawer({ 1,(int)gX * (int)gY });

		
	}

	cmd(Islands, int count; int skipper; pMode mode; int r; int g; int b;)
	{
		reflect;

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		vs::islands = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)in.mode,
				.skipper = in.skipper,
				.base_color = float4(in.r / 100.,in.g / 100.,in.b / 100.,1)
			},
		};

		vs::islands.set();

		Drawer::NullDrawer({ 1,(int)gX * (int)gY });

		
	}

	cmd(Waterfall, int count, int skipper, pMode mode, int r, int g, int b)
	{
		reflect;

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		vs::waterfall = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)in.mode,
				.skipper = in.skipper,
				.base_color = float4(in.r / 100.,in.g / 100.,in.b / 100.,1)
			},
		};

		vs::waterfall.set();

		Drawer::NullDrawer({ 1,(int)gX * (int)gY });

		
	}

	void DoubleStar(int count, int skipper, pMode mode)
	{
		int gX = sqrt(count / skipper);
		int gY = sqrt(count / skipper);

		psModeSet(mode);

		vs::fish= {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)mode,
				.skipper = skipper,
			},
		};

		vs::fish.set();

		Drawer::NullDrawer({ 1, (int)gX * (int)gY });
	}

	void Tree(int count, int skipper, pMode mode)
	{
		int gX = sqrt(count / skipper);
		int gY = sqrt(count / skipper);

		psModeSet(mode);

		vs::tree = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)mode,
				.skipper = skipper,
			},
		};

		vs::tree.set();

		Drawer::NullDrawer({1,(int)gX*(int)gY});
	}

	void Libra_spheres(int count, int skipper, pMode mode)
	{
		int gX = sqrt(count / skipper);
		int gY = sqrt(count / skipper);

		psModeSet(mode);

		vs::libra_sph = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)mode,
				.skipper = skipper,
			},
		};

		vs::libra_sph.set();

		Drawer::NullDrawer({ 1, (int)gX * (int)gY });
	}

	void Pillars(int count,int skipper, pMode mode)
	{
		int gX = sqrt(count / skipper);
		int gY = sqrt(count / skipper);

		psModeSet(mode);

		vs::pillars = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)mode,
				.skipper = skipper,
			},
		};

		vs::pillars.set();

		Drawer::NullDrawer({ 1, (int)gX * (int)gY });
	}

	void OuterSpace(int count, int skipper, pMode mode)
	{
		int gX = sqrt(count / skipper);
		int gY = sqrt(count / skipper);

		psModeSet(mode);

		vs::space = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)mode,
				.skipper = skipper,
			},
		};

		vs::space.set();

		Drawer::NullDrawer({1,(int)gX*(int)gY});
	}

	void NeutronStar(int count, int skipper, pMode mode)
	{
		int gX = sqrt(count / skipper);
		int gY = sqrt(count / skipper);

		psModeSet(mode);

		vs::neitron_star = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)mode,
				.skipper = skipper,
			},
		};

		vs::neitron_star.set();

		Drawer::NullDrawer({ 1, (int)gX * (int)gY });
	}

	cmd(DoubleTwo, int count, int skipper, pMode mode, int r, int g, int b)
	{
		reflect;
		float r = in.r / 100.f;
		float g = in.g / 100.f;
		float b = in.b / 100.f;
		float4 base_color = float4(r, g, b, 1);

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		vs::galaxy_2 = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)in.mode,
				.skipper = in.skipper,
				.base_color = base_color
			},
		};

		vs::galaxy_2.set();

		Drawer::NullDrawer({ 1,(int)gX * (int)gY });
		
	}

	cmd(Galaxy, int count, int skipper, pMode mode, int r, int g, int b)
	{
		reflect;
		float r = in.r / 100.f;
		float g = in.g / 100.f;
		float b = in.b / 100.f;
		float4 base_color = float4(r,g,b, 1);

		int gX = sqrt(in.count / in.skipper);
		int gY = sqrt(in.count / in.skipper);

		psModeSet(in.mode);

		vs::galaxy = {
			.params = {
				.model = XMMatrixTranspose(XMMatrixTranslation(0,0,0)),
				.gX = gX,
				.gY = gY,
				.mode = (int)in.mode,
				.skipper = in.skipper,
				.base_color = base_color
			},
		};

		vs::galaxy.set();

		Drawer::NullDrawer({1,(int)gX*(int)gY});
		
	}

	cmd(Libra, int quality)
	{
		reflect;

		int pillars_cnt = 1000 * 1000;
		int outerSpace_cnt = 6853 / in.quality;
		int galaxy_cnt = 182361 / in.quality;

		//hi
		RenderTarget::Set({ texture::pBuf,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		Tree(pillars_cnt, 1, pMode::point);
		Libra_spheres(256 * 256, 1, pMode::point);
		InsideNebula({pillars_cnt,1,pMode::point,100,252,400});
		OuterSpace(outerSpace_cnt, 1, pMode::point);
		Galaxy({galaxy_cnt,14,pMode::point,254,220,41});

		//mid
		RenderTarget::Set({texture::pBufMid,0});
		RenderTarget::Clear({ 0,0,0,0 });
		Galaxy({ galaxy_cnt, 4, pMode::glow ,254,220,41 });

		//low
		RenderTarget::Set({ texture::pBufLow,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		Tree(pillars_cnt, 1394 / 2, pMode::glow);
		InsideNebula({pillars_cnt,1394,pMode::glow,100,202,400});
		Libra_spheres(256 * 256, 143, pMode::glow);
			OuterSpace(outerSpace_cnt, 64, pMode::glow);

		
	}

	cmd(Aquarius, int quality)
	{
		reflect;

		int pillars_cnt = 2000 * 1000;
		int outerSpace_cnt = 6853 / in.quality;
		int galaxy_cnt = 182361 / in.quality;

		//hi
		RenderTarget::Set({ texture::pBuf,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		Blob({ pillars_cnt,1,pMode::point,100,252,500 });
		Islands({ pillars_cnt/2,1,pMode::point,130,112,10 });
		Waterfall({ pillars_cnt / 4,1,pMode::point,30,352,1100 });
		OuterSpace(outerSpace_cnt, 1, pMode::point);
//		Galaxy({ galaxy_cnt,14,pMode::point,254,220,41 });

		//mid
		RenderTarget::Set({ texture::pBufMid,0 });
		RenderTarget::Clear({ 0,0,0,0 });
		Blob({ pillars_cnt,194,pMode::glow,100,252,600 });

		//low
		RenderTarget::Set({ texture::pBufLow,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		//Blob({ pillars_cnt,1394,pMode::glow,100,252,600 });
		OuterSpace(outerSpace_cnt, 64, pMode::glow);

		
	}

	cmd(Crab, int quality)
	{
		reflect;

		int pillars_cnt = 2000 * 1000;
		int outerSpace_cnt = 6853 / in.quality;
		int galaxy_cnt = 182361 / in.quality;

		//hi
		RenderTarget::Set({ texture::pBuf,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		Pearl({ pillars_cnt,1,pMode::point,600,252,100 });
		InsideNebula({ pillars_cnt / 2, 1, pMode::point ,1500,100,00 });
		//Islands({ pillars_cnt / 2,1,pMode::point,130,112,10 });
		//Waterfall({ pillars_cnt / 4,1,pMode::point,30,352,1100 });
		OuterSpace(outerSpace_cnt, 1, pMode::point);
		//		Galaxy({ galaxy_cnt,14,pMode::point,254,220,41 });

				//mid
		RenderTarget::Set({ texture::pBufMid,0 });
		RenderTarget::Clear({ 0,0,0,0 });
		Pearl({ pillars_cnt,194,pMode::glow,600,252,100 });

		//low
		RenderTarget::Set({ texture::pBufLow,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		//Blob({ pillars_cnt,1394,pMode::glow,100,252,600 });
		OuterSpace(outerSpace_cnt, 64, pMode::glow);

		
	}

	cmd(LeoBigStar, int quality)
	{
		reflect;

		int pillars_cnt = 2000 * 1000;
		int outerSpace_cnt = 6853 / in.quality;
		int galaxy_cnt = 182361 / in.quality;

		//hi
		RenderTarget::Set({ texture::pBuf,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		LeoStar({ pillars_cnt,1,pMode::point,600,252,100 });
		//InsideNebula({ pillars_cnt / 2, 1, pMode::point ,1500,100,00 });
		//Islands({ pillars_cnt / 2,1,pMode::point,130,112,10 });
		//Waterfall({ pillars_cnt / 4,1,pMode::point,30,352,1100 });
		OuterSpace(outerSpace_cnt, 1, pMode::point);
		//		Galaxy({ galaxy_cnt,14,pMode::point,254,220,41 });

				//mid
		RenderTarget::Set({ texture::pBufMid,0 });
		RenderTarget::Clear({ 0,0,0,0 });
		LeoStar({ pillars_cnt,194,pMode::glow,600,252,100 });

		//low
		RenderTarget::Set({ texture::pBufLow,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		//Blob({ pillars_cnt,1394,pMode::glow,100,252,600 });
		OuterSpace(outerSpace_cnt, 64, pMode::glow);

		
	}

	cmd(Capri, int quality)
	{
		reflect;

		int pillars_cnt = 2000 * 1000;
		int outerSpace_cnt = 6853 / in.quality;
		int galaxy_cnt = 182361 / in.quality;

		//hi
		RenderTarget::Set({ texture::pBuf,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		CapStar({ pillars_cnt,1,pMode::point,100,252,1400 });
		//InsideNebula({ pillars_cnt / 2, 1, pMode::point ,1500,100,00 });
		//Islands({ pillars_cnt / 2,1,pMode::point,130,112,10 });
		//Waterfall({ pillars_cnt / 4,1,pMode::point,30,352,1100 });
		OuterSpace(outerSpace_cnt, 1, pMode::point);
		//		Galaxy({ galaxy_cnt,14,pMode::point,254,220,41 });

				//mid
		RenderTarget::Set({ texture::pBufMid,0 });
		RenderTarget::Clear({ 0,0,0,0 });
		CapStar({ pillars_cnt,194,pMode::glow,100,252,1400 });

		//low
		RenderTarget::Set({ texture::pBufLow,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		//Blob({ pillars_cnt,1394,pMode::glow,100,252,600 });
		OuterSpace(outerSpace_cnt, 64, pMode::glow);

		
	}

	cmd(Taurus, int quality)
	{
		reflect;

		int pillars_cnt = 2000 * 1000;
		int outerSpace_cnt = 6853 / in.quality;
		int galaxy_cnt = 182361 / in.quality;

		//hi
		RenderTarget::Set({ texture::pBuf,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		Tau({ pillars_cnt,1,pMode::point,100,252,1400 });
		//InsideNebula({ pillars_cnt / 2, 1, pMode::point ,1500,100,00 });
		//Islands({ pillars_cnt / 2,1,pMode::point,130,112,10 });
		//Waterfall({ pillars_cnt / 4,1,pMode::point,30,352,1100 });
		OuterSpace(outerSpace_cnt, 1, pMode::point);
		//		Galaxy({ galaxy_cnt,14,pMode::point,254,220,41 });

				//mid
		RenderTarget::Set({ texture::pBufMid,0 });
		RenderTarget::Clear({ 0,0,0,0 });
		Tau({ pillars_cnt,194,pMode::glow,100,252,1400 });

		//low
		RenderTarget::Set({ texture::pBufLow,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		//Blob({ pillars_cnt,1394,pMode::glow,100,252,600 });
		OuterSpace(outerSpace_cnt, 64, pMode::glow);

		
	}


	
	


	cmd(Girl, int quality, int xPos, int yPos, int zPos, int brightness, int tickness, switcher stencil)
	{
		reflect;

		//Object::Capri({ .quality = 1 });

		int pillars_cnt2 = 2000 * 1000;

		int pillars_cnt = 3725442 / in.quality;
		int outerSpace_cnt = 6853 / in.quality;
		int neutronStar_cnt = 279620 / in.quality;
		int galaxy_cnt = 182361 / in.quality;

		//hi
		RenderTarget::Set({ texture::pBuf,0 });

		//vrg({ pillars_cnt/2,1,pMode::point,1390,925,111 });
		Maze({ 200000,1,pMode::point,1390,925,111 });

		//OuterSpace(outerSpace_cnt, 1, pMode::point);
		//NeutronStar(neutronStar_cnt, 1, pMode::point);

		//Galaxy({ galaxy_cnt, 14, pMode::point ,100,200,300 });

		//RenderTarget::Set({ texture::pBuf,0 });
		//RenderTarget::Clear({ 0,0,0,0 });

		//call show obj

		Culling::Set({ cullmode::off });
		DepthBuf::Mode({ depthmode::off });

		//mid
		RenderTarget::Set({ texture::pBufMid,0 });
		//vrg({ pillars_cnt,94,pMode::glow,20,30,75 });
		Maze({ 200000,94,pMode::glow,20,30,75 });

		//Galaxy({ galaxy_cnt, 4, pMode::glow ,100,200,300 });

		//low
		RenderTarget::Set({ texture::pBufLow,0 });


		//Pillars(pillars_cnt, 10394, pMode::glow);
		OuterSpace(outerSpace_cnt, 64, pMode::glow);

		//------------------
		//hi
		

		
	}

	cmd(Scorpio, int quality)
	{
		reflect;

		int pillars_cnt = 2000 * 1000;
		int outerSpace_cnt = 6853 / in.quality;
		int galaxy_cnt = 182361 / in.quality;

		//hi
		RenderTarget::Set({ texture::pBuf,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		
		ScorpBall({ pillars_cnt/2,1,pMode::point,1390,925,111 });
		Nebula2({ pillars_cnt,1,pMode::point,1390,925,111 });
		//InsideNebula({ pillars_cnt , 1, pMode::point ,220,130,175 });
		//Islands({ pillars_cnt / 2,1,pMode::point,130,112,10 });
		//Waterfall({ pillars_cnt / 4,1,pMode::point,30,352,1100 });
		OuterSpace(outerSpace_cnt, 1, pMode::point);
		//		Galaxy({ galaxy_cnt,14,pMode::point,254,220,41 });

				//mid
		RenderTarget::Set({ texture::pBufMid,0 });
		RenderTarget::Clear({ 0,0,0,0 });
		
		//ScorpBall({ pillars_cnt,1,pMode::point,1390,925,111 });
		//ScorpBall({ pillars_cnt,94,pMode::glow,1,10,5 });
		Nebula2({ pillars_cnt,94,pMode::glow,20,30,75 });
		//InsideNebula({ pillars_cnt , 134, pMode::glow ,40,16,10 });

		//low
		RenderTarget::Set({ texture::pBufLow,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		//Blob({ pillars_cnt,1394,pMode::glow,100,252,600 });
		OuterSpace(outerSpace_cnt, 64, pMode::glow);

		
	}

	cmd(Virgo, int quality)
	{
		reflect;

		int pillars_cnt = 2000 * 1000;
		int outerSpace_cnt = 6853 / in.quality;
		int galaxy_cnt = 182361 / in.quality;

		//hi
		RenderTarget::Set({ texture::pBuf,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		vrg({ pillars_cnt,1,pMode::point,1390,925,111 });
		OuterSpace(outerSpace_cnt, 1, pMode::point);
		//Galaxy({ galaxy_cnt,14,pMode::point,254,220,41 });

		//mid
		RenderTarget::Set({ texture::pBufMid,0 });
		RenderTarget::Clear({ 0,0,0,0 });
		vrg({ pillars_cnt,94,pMode::glow,20,30,75 });

		//low
		RenderTarget::Set({ texture::pBufLow,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		OuterSpace(outerSpace_cnt, 64, pMode::glow);

		
	}

	cmd(Aries, int quality)
	{
		reflect;

		int pillars_cnt = 2000 * 1000;
		int outerSpace_cnt = 6853 / in.quality;
		int galaxy_cnt = 182361 / in.quality;

		//hi
		RenderTarget::Set({ texture::pBuf,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		Rocks({ pillars_cnt,1,pMode::point,1390,925,111 });
		//InsideNebula({ pillars_cnt , 1, pMode::point ,220,130,175 });
		//Islands({ pillars_cnt / 2,1,pMode::point,130,112,10 });
		//Waterfall({ pillars_cnt / 4,1,pMode::point,30,352,1100 });
		OuterSpace(outerSpace_cnt, 1, pMode::point);
		//		Galaxy({ galaxy_cnt,14,pMode::point,254,220,41 });

				//mid
		RenderTarget::Set({ texture::pBufMid,0 });
		RenderTarget::Clear({ 0,0,0,0 });
		Rocks({ pillars_cnt,194,pMode::glow,20,30,75 });
		//InsideNebula({ pillars_cnt , 134, pMode::glow ,40,16,10 });

		//low
		RenderTarget::Set({ texture::pBufLow,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		//Blob({ pillars_cnt,1394,pMode::glow,100,252,600 });
		OuterSpace(outerSpace_cnt, 64, pMode::glow);

		
	}

	cmd(Twins, int quality)
	{
		reflect;

		int pillars_cnt = 2000 * 1000;
		int outerSpace_cnt = 6853 / in.quality;
		int galaxy_cnt = 182361 / in.quality;

		//hi
		RenderTarget::Set({ texture::pBuf,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		Transporter({ pillars_cnt,1,pMode::point,1390,925,111 });
		InsideNebula({ pillars_cnt , 1, pMode::point ,220,130,175 });
		//Islands({ pillars_cnt / 2,1,pMode::point,130,112,10 });
		//Waterfall({ pillars_cnt / 4,1,pMode::point,30,352,1100 });
		OuterSpace(outerSpace_cnt, 1, pMode::point);
		//		Galaxy({ galaxy_cnt,14,pMode::point,254,220,41 });

				//mid
		RenderTarget::Set({ texture::pBufMid,0 });
		RenderTarget::Clear({ 0,0,0,0 });
		Transporter({ pillars_cnt,194,pMode::glow,2,3,7 });
		InsideNebula({ pillars_cnt , 134, pMode::glow ,10,26,40 });

		//low
		RenderTarget::Set({ texture::pBufLow,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		//Blob({ pillars_cnt,1394,pMode::glow,100,252,600 });
		OuterSpace(outerSpace_cnt, 64, pMode::glow);

		
	}

	cmd(Fish, int quality)
	{
		reflect;

		int pillars_cnt = 3725470 / in.quality;
		int outerSpace_cnt = 6853 / in.quality;
		int galaxy_cnt = 182361 / in.quality;
		int galaxy_cnt2 = 2182361 / in.quality;

		//hi
		RenderTarget::Set({ texture::pBuf,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		//DoubleStar(pillars_cnt, 1, pMode::point);
		DoubleTwo({ pillars_cnt, 1, pMode::point,90,130,800 });
		//InsideNebula({ pillars_cnt / 2, 1, pMode::point ,100,200,600});
		OuterSpace(outerSpace_cnt, 1, pMode::point);

		//mid
		RenderTarget::Set({ texture::pBufMid,0 });
		RenderTarget::Clear({ 0,0,0,0 });
		DoubleTwo({ galaxy_cnt, 25, pMode::glow, 100, 200, 300 });

		//low
		RenderTarget::Set({ texture::pBufLow,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		DoubleTwo({ galaxy_cnt, 2, pMode::glow,20,40,160 });
		
		//InsideNebula({pillars_cnt / 2, 1394, pMode::glow, 100, 200, 600});
			OuterSpace(outerSpace_cnt, 64, pMode::glow);

		
	}

	cmd(Zenith, int quality)
	{
		reflect;

		int pillars_cnt = 3725470/2 / in.quality;
		int outerSpace_cnt = 6853 / in.quality;
		int galaxy_cnt = 182361 / in.quality;

		//hi
		RenderTarget::Set({ texture::pBuf,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		PillarsHand(pillars_cnt, 1, pMode::point);
		InsideNebula({ pillars_cnt, 1, pMode::point,100,200,600 });
		OuterSpace(outerSpace_cnt, 1, pMode::point);

		//mid
		RenderTarget::Set({ texture::pBufMid,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		//low
		RenderTarget::Set({ texture::pBufLow,0 });
		RenderTarget::Clear({ 0,0,0,0 });

		PillarsHand(pillars_cnt, 1394/2, pMode::glow);
		InsideNebula({ pillars_cnt, 1394, pMode::glow ,100,200,600});
	//	OuterSpace(outerSpace_cnt, 64, pMode::glow);

		
	}

	cmd(Saggitarius, int quality)
	{
		reflect;

		int pillars_cnt = 3725470/in.quality;
		int outerSpace_cnt = 6853 / in.quality;
		int neutronStar_cnt = 279620 / in.quality;
		int galaxy_cnt = 182361 / in.quality;

		//hi
		RenderTarget::Set({ texture::pBuf,0 });
		RenderTarget::Clear({ 0,0,0,0 });

			Pillars(pillars_cnt,1,pMode::point);
			OuterSpace(outerSpace_cnt, 1, pMode::point);
			NeutronStar(neutronStar_cnt, 1, pMode::point);
			
			Galaxy({ galaxy_cnt, 14, pMode::point ,100,200,300 });

		//mid
			RenderTarget::Set({ texture::pBufMid,0 });
			RenderTarget::Clear({ 0,0,0,0 });


			Galaxy({ galaxy_cnt, 4, pMode::glow ,100,200,300 });

		//low
		RenderTarget::Set({ texture::pBufLow,0 });
		RenderTarget::Clear({ 0,0,0,0 });

			Pillars(pillars_cnt, 10394, pMode::glow);
			OuterSpace(outerSpace_cnt, 64, pMode::glow);
			//NeutronStar(1024 * 1024, 1, pMode::glow);
			//Galaxy(182361, 4, pMode::glow);

		
	}

	cmd(CalcNormals, texture srcGeomerty, texture targetNrml)
	{
		reflect;

		RenderTarget::Set({ in.targetNrml, 0 });

		vs::quad.set();

		ps::genNormals = {

			.textures = {
				.geo = in.srcGeomerty
			},

			.samplers = {
				.sam1Filter = filter::linear,
				.sam1AddressU = addr::wrap,
				.sam1AddressV = addr::wrap
			}
		};

		ps::genNormals.set();

		Drawer::NullDrawer({ 1, 1 });
		RenderTarget::GenerateMips({});

		

	}

	cmd(Calc, texture targetGeo, texture targetNrml)
	{
		reflect;

		BlendMode::Set({ blendmode::off, blendop::add });
		Culling::Set({cullmode::off});
		RenderTarget::Set({ in.targetGeo,0 });
		DepthBuf::Mode({ depthmode::off });

		//pos
		vs::quad.set();
		ps::cat.set();
		Drawer::NullDrawer({ 1, 1 });
		RenderTarget::GenerateMips({});

		//normals
		CalcNormals({ in.targetGeo, in.targetNrml });

		
	}

}
