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
		

		// ============================================================
// ЛИНИЯ 1
// ============================================================
		NewLine();
		AddPointToLine({ 0,   0,   0 });
		AddPointToLine({ 7,   0,   -17 });
		AddPointToLine({ 26,   0,   -29 });
		AddPointToLine({ 93,   0,   -53 });
		AddPointToLine({ 154,   0,   -1 });
// 1A

		NewLine();
		AddPointToLine({ 0,   0,   0 });
		AddPointToLine({ 18,  -2,  12 });
		AddPointToLine({ 32,  -5,  25 });
		AddPointToLine({ 30,  -4,  38 });
		AddPointToLine({ 25,  -1,  50 });
		AddPointToLine({ 10,   2,  63 });
		AddPointToLine({ -15,   4,  75 });
		AddPointToLine({ -24,   3,  90 });
		AddPointToLine({ -20,  -3, 105 });

		// 1B
		NewLine();
		AddPointToLine({ -25,   3,  95 });
		AddPointToLine({ -20,  -1, 110 });
		AddPointToLine({ 0,  -6, 125 });
		AddPointToLine({ 20,  -4, 140 });
		AddPointToLine({ 30,   3, 155 });
		AddPointToLine({ 15,   6, 170 });
		AddPointToLine({ -12,   5, 185 });
		AddPointToLine({ -22,   0, 198 });
		AddPointToLine({ -18,  -6, 210 });

		// 1C
		NewLine();
		AddPointToLine({ -28,  -1, 200 });
		AddPointToLine({ -18,  -4, 215 });
		AddPointToLine({ 10,  -7, 230 });
		AddPointToLine({ 27,  -3, 245 });
		AddPointToLine({ 25,   3, 260 });
		AddPointToLine({ 5,   6, 275 });
		AddPointToLine({ -25,   4, 290 });
		AddPointToLine({ -28,   0, 303 });
		AddPointToLine({ -10,  -6, 315 });

		// 1D
		NewLine();
		AddPointToLine({ -30,  -2, 305 });
		AddPointToLine({ -18,  -5, 320 });
		AddPointToLine({ 18,  -6, 335 });
		AddPointToLine({ 30,   0, 350 });
		AddPointToLine({ 15,   6, 365 });
		AddPointToLine({ -15,   6, 380 });
		AddPointToLine({ -30,   1, 395 });
		AddPointToLine({ -25,  -3, 408 });
		AddPointToLine({ 10,  -7, 420 });

		// 1E
		NewLine();
		AddPointToLine({ -20,  -5, 410 });
		AddPointToLine({ 8,  -7, 425 });
		AddPointToLine({ 28,  -4, 440 });
		AddPointToLine({ 25,   2, 455 });
		AddPointToLine({ 0,   7, 470 });
		AddPointToLine({ -25,   4, 485 });
		AddPointToLine({ -30,  -2, 500 });
		AddPointToLine({ -20,  -5, 513 });
		AddPointToLine({ -10,  -6, 525 });

		// 1F
		NewLine();
		AddPointToLine({ -10,  -6, 515 });
		AddPointToLine({ 8,  -5, 530 });
		AddPointToLine({ 30,   0, 545 });
		AddPointToLine({ 20,   5, 560 });
		AddPointToLine({ -15,   5, 575 });
		AddPointToLine({ -12,   1, 588 });
		AddPointToLine({ 0,  -5, 600 });


		// ============================================================
		// ЛИНИЯ 2
		// ============================================================

		// 2A
		NewLine();
		AddPointToLine({ 0,   0,   0 });
		AddPointToLine({ -18,   2,  15 });
		AddPointToLine({ -30,   5,  30 });
		AddPointToLine({ -22,   6,  45 });
		AddPointToLine({ 5,   3,  60 });
		AddPointToLine({ 22,  -1,  75 });
		AddPointToLine({ 22,  -5,  90 });
		AddPointToLine({ 10,  -6, 102 });
		AddPointToLine({ 0,  -5, 110 });

		// 2B
		NewLine();
		AddPointToLine({ 22,  -5, 100 });
		AddPointToLine({ 5,  -5, 115 });
		AddPointToLine({ -22,  -2, 130 });
		AddPointToLine({ -30,   3, 145 });
		AddPointToLine({ -15,   6, 160 });
		AddPointToLine({ 12,   5, 175 });
		AddPointToLine({ 30,   0, 190 });
		AddPointToLine({ 25,  -3, 203 });
		AddPointToLine({ 15,  -5, 215 });

		// 2C
		NewLine();
		AddPointToLine({ -15,  -6, 205 });
		AddPointToLine({ -25,  -5, 220 });
		AddPointToLine({ -30,  -2, 235 });
		AddPointToLine({ -20,   3, 250 });
		AddPointToLine({ 10,   7, 265 });
		AddPointToLine({ 28,   3, 280 });
		AddPointToLine({ 22,  -5, 295 });
		AddPointToLine({ 5,  -7, 308 });
		AddPointToLine({ -5,  -7, 320 });

		// 2D
		NewLine();
		AddPointToLine({ -28,  -3, 310 });
		AddPointToLine({ -30,   0, 325 });
		AddPointToLine({ -25,   3, 340 });
		AddPointToLine({ 0,   7, 355 });
		AddPointToLine({ 25,   4, 370 });
		AddPointToLine({ 30,  -2, 385 });
		AddPointToLine({ 10,  -6, 400 });
		AddPointToLine({ -10,  -6, 413 });
		AddPointToLine({ -20,  -5, 425 });

		// 2E
		NewLine();
		AddPointToLine({ -30,   1, 415 });
		AddPointToLine({ -25,   5, 430 });
		AddPointToLine({ -15,   6, 445 });
		AddPointToLine({ 5,   4, 460 });
		AddPointToLine({ 30,   0, 475 });
		AddPointToLine({ 18,  -5, 490 });
		AddPointToLine({ -12,  -6, 505 });
		AddPointToLine({ -25,  -4, 518 });
		AddPointToLine({ -30,  -1, 530 });

		// 2F
		NewLine();
		AddPointToLine({ -30,  -1, 520 });
		AddPointToLine({ -15,   3, 535 });
		AddPointToLine({ 5,   7, 550 });
		AddPointToLine({ 25,   4, 565 });
		AddPointToLine({ 30,  -1, 580 });
		AddPointToLine({ 15,  -5, 590 });
		AddPointToLine({ 0,  -5, 600 });


		// ============================================================
		// ЛИНИЯ 3
		// ============================================================

		// 3A
		NewLine();
		AddPointToLine({ 0,   0,   0 });
		AddPointToLine({ 12,   5,  16 });
		AddPointToLine({ 25,   5,  32 });
		AddPointToLine({ 30,   1,  48 });
		AddPointToLine({ 18,  -4,  65 });
		AddPointToLine({ -10,  -5,  82 });
		AddPointToLine({ -28,  -2,  95 });
		AddPointToLine({ -27,   1, 105 });
		AddPointToLine({ -22,   4, 115 });

		// 3B
		NewLine();
		AddPointToLine({ -28,  -2, 105 });
		AddPointToLine({ -22,   2, 115 });
		AddPointToLine({ 5,   6, 130 });
		AddPointToLine({ 25,   3, 145 });
		AddPointToLine({ 28,  -5, 160 });
		AddPointToLine({ 5,  -6, 175 });
		AddPointToLine({ -22,  -3, 190 });
		AddPointToLine({ -30,   2, 205 });
		AddPointToLine({ -10,   7, 220 });

		// 3C
		NewLine();
		AddPointToLine({ -30,   4, 210 });
		AddPointToLine({ -10,   6, 220 });
		AddPointToLine({ 20,   5, 235 });
		AddPointToLine({ 30,  -1, 250 });
		AddPointToLine({ 12,  -6, 265 });
		AddPointToLine({ -20,  -5, 280 });
		AddPointToLine({ -30,   1, 295 });
		AddPointToLine({ -22,   5, 308 });
		AddPointToLine({ -12,   7, 320 });

		// 3D
		NewLine();
		AddPointToLine({ -30,   1, 310 });
		AddPointToLine({ 0,   5, 325 });
		AddPointToLine({ 30,  -1, 340 });
		AddPointToLine({ 10,  -6, 355 });
		AddPointToLine({ -22,  -5, 370 });
		AddPointToLine({ -30,   2, 385 });
		AddPointToLine({ -10,   7, 400 });
		AddPointToLine({ 15,   6, 413 });
		AddPointToLine({ 20,   5, 425 });

		// 3E
		NewLine();
		AddPointToLine({ 30,  -1, 415 });
		AddPointToLine({ 20,  -5, 430 });
		AddPointToLine({ 12,  -6, 445 });
		AddPointToLine({ -10,  -4, 460 });
		AddPointToLine({ -30,   1, 475 });
		AddPointToLine({ -12,   5, 490 });
		AddPointToLine({ 20,   5, 505 });
		AddPointToLine({ 30,   1, 518 });
		AddPointToLine({ 30,  -1, 530 });

		// 3F
		NewLine();
		AddPointToLine({ 10,  -6, 520 });
		AddPointToLine({ 0,  -5, 530 });
		AddPointToLine({ -20,  -5, 545 });
		AddPointToLine({ -28,  -1, 560 });
		AddPointToLine({ -12,   6, 575 });
		AddPointToLine({ 10,   1, 588 });
		AddPointToLine({ 0,  -5, 600 });



		// ============================================================
// ЛИНИЯ 1 — РАНДОМИЗИРОВАННАЯ
// ============================================================

// 1A
		NewLine();
		AddPointToLine({ 150,   0,   0 });
		AddPointToLine({ 171,  -4,  11 });
		AddPointToLine({ 186,   3,  24 });
		AddPointToLine({ 179,   7,  39 });
		AddPointToLine({ 162,   1,  51 });
		AddPointToLine({ 143,  -5,  66 });
		AddPointToLine({ 121,   2,  78 });
		AddPointToLine({ 129,   8,  91 });
		AddPointToLine({ 137,  -2, 105 });

		// 1B
		NewLine();
		AddPointToLine({ 131,   5,  96 });
		AddPointToLine({ 139,  -3, 111 });
		AddPointToLine({ 158,  -7, 124 });
		AddPointToLine({ 176,   1, 139 });
		AddPointToLine({ 184,   9, 153 });
		AddPointToLine({ 169,   4, 171 });
		AddPointToLine({ 145,  -4, 184 });
		AddPointToLine({ 122,   2, 199 });
		AddPointToLine({ 136,  -8, 211 });

		// 1C
		NewLine();
		AddPointToLine({ 128,   3, 201 });
		AddPointToLine({ 139,  -6, 216 });
		AddPointToLine({ 166,  -9, 228 });
		AddPointToLine({ 181,  -1, 246 });
		AddPointToLine({ 174,   8, 259 });
		AddPointToLine({ 151,   3, 276 });
		AddPointToLine({ 126,  -5, 289 });
		AddPointToLine({ 119,   1, 304 });
		AddPointToLine({ 143,  -7, 316 });

		// 1D
		NewLine();
		AddPointToLine({ 124,  -2, 306 });
		AddPointToLine({ 141,  -8, 321 });
		AddPointToLine({ 172,  -4, 334 });
		AddPointToLine({ 181,   5, 351 });
		AddPointToLine({ 169,  10, 364 });
		AddPointToLine({ 146,   5, 381 });
		AddPointToLine({ 123,  -3, 394 });
		AddPointToLine({ 130,  -6, 409 });
		AddPointToLine({ 158,  -9, 421 });

		// 1E
		NewLine();
		AddPointToLine({ 136,  -4, 411 });
		AddPointToLine({ 158,  -8, 426 });
		AddPointToLine({ 181,   0, 439 });
		AddPointToLine({ 177,   7, 456 });
		AddPointToLine({ 154,   9, 469 });
		AddPointToLine({ 128,   1, 486 });
		AddPointToLine({ 116,  -7, 499 });
		AddPointToLine({ 135,  -3, 514 });
		AddPointToLine({ 146,  -8, 526 });

		// 1F
		NewLine();
		AddPointToLine({ 141,  -5, 516 });
		AddPointToLine({ 165,  -7, 531 });
		AddPointToLine({ 184,   2, 544 });
		AddPointToLine({ 176,   8, 561 });
		AddPointToLine({ 149,   4, 574 });
		AddPointToLine({ 132,  -4, 589 });
		AddPointToLine({ 150,  -6, 600 });


		// ============================================================
		// ЛИНИЯ 2 — РАНДОМИЗИРОВАННАЯ
		// ============================================================

		// 2A
		NewLine();
		AddPointToLine({ 150,   0,   0 });
		AddPointToLine({ 129,   5,  14 });
		AddPointToLine({ 116,   2,  29 });
		AddPointToLine({ 122,   8,  44 });
		AddPointToLine({ 149,   4,  61 });
		AddPointToLine({ 176,  -3,  74 });
		AddPointToLine({ 180,  -8,  89 });
		AddPointToLine({ 164,  -2, 103 });
		AddPointToLine({ 151,  -7, 111 });

		// 2B
		NewLine();
		AddPointToLine({ 171,  -4, 101 });
		AddPointToLine({ 156,  -8, 116 });
		AddPointToLine({ 132,  -1, 129 });
		AddPointToLine({ 116,   6, 146 });
		AddPointToLine({ 129,   9, 159 });
		AddPointToLine({ 158,   3, 176 });
		AddPointToLine({ 183,  -2, 189 });
		AddPointToLine({ 179,  -7, 204 });
		AddPointToLine({ 161,  -4, 216 });

		// 2C
		NewLine();
		AddPointToLine({ 132,  -5, 206 });
		AddPointToLine({ 119,  -7, 221 });
		AddPointToLine({ 114,   1, 234 });
		AddPointToLine({ 127,   8, 251 });
		AddPointToLine({ 156,   6, 264 });
		AddPointToLine({ 179,  -1, 281 });
		AddPointToLine({ 171,  -8, 294 });
		AddPointToLine({ 149,  -4, 309 });
		AddPointToLine({ 137,  -7, 321 });

		// 2D
		NewLine();
		AddPointToLine({ 122,  -2, 309 });
		AddPointToLine({ 117,   4, 324 });
		AddPointToLine({ 131,   8, 341 });
		AddPointToLine({ 157,   5, 354 });
		AddPointToLine({ 181,  -1, 371 });
		AddPointToLine({ 177,  -7, 384 });
		AddPointToLine({ 157,  -8, 401 });
		AddPointToLine({ 138,  -2, 412 });
		AddPointToLine({ 129,  -6, 426 });

		// 2E
		NewLine();
		AddPointToLine({ 131,   2, 416 });
		AddPointToLine({ 131,   8, 429 });
		AddPointToLine({ 144,   5, 446 });
		AddPointToLine({ 166,   1, 459 });
		AddPointToLine({ 183,  -4, 476 });
		AddPointToLine({ 171,  -8, 489 });
		AddPointToLine({ 144,  -3, 506 });
		AddPointToLine({ 121,  -6, 519 });
		AddPointToLine({ 130,  -4, 531 });

		// 2F
		NewLine();
		AddPointToLine({ 124,  -1, 521 });
		AddPointToLine({ 139,   6, 534 });
		AddPointToLine({ 163,   8, 551 });
		AddPointToLine({ 181,   1, 564 });
		AddPointToLine({ 176,  -6, 581 });
		AddPointToLine({ 159,  -7, 591 });
		AddPointToLine({ 150,  -5, 600 });


		// ============================================================
		// ЛИНИЯ 3 — РАНДОМИЗИРОВАННАЯ
		// ============================================================

		// 3A
		NewLine();
		AddPointToLine({ 150,   0,   0 });
		AddPointToLine({ 165,   8,  15 });
		AddPointToLine({ 179,   2,  31 });
		AddPointToLine({ 184,  -4,  47 });
		AddPointToLine({ 168,  -7,  64 });
		AddPointToLine({ 142,  -3,  81 });
		AddPointToLine({ 119,   3,  94 });
		AddPointToLine({ 126,   7, 106 });
		AddPointToLine({ 138,   2, 116 });

		// 3B
		NewLine();
		AddPointToLine({ 121,  -3, 106 });
		AddPointToLine({ 134,   5, 114 });
		AddPointToLine({ 159,   8, 131 });
		AddPointToLine({ 178,   1, 144 });
		AddPointToLine({ 181,  -8, 161 });
		AddPointToLine({ 156,  -5, 174 });
		AddPointToLine({ 128,   2, 191 });
		AddPointToLine({ 119,   7, 204 });
		AddPointToLine({ 143,   9, 221 });

		// 3C
		NewLine();
		AddPointToLine({ 118,   3, 211 });
		AddPointToLine({ 141,   8, 219 });
		AddPointToLine({ 171,   3, 236 });
		AddPointToLine({ 182,  -5, 249 });
		AddPointToLine({ 165,  -9, 266 });
		AddPointToLine({ 136,  -2, 279 });
		AddPointToLine({ 117,   5, 296 });
		AddPointToLine({ 124,   8, 307 });
		AddPointToLine({ 141,   4, 321 });

		// 3D
		NewLine();
		AddPointToLine({ 119,   1, 311 });
		AddPointToLine({ 146,   7, 326 });
		AddPointToLine({ 179,   2, 339 });
		AddPointToLine({ 166,  -7, 356 });
		AddPointToLine({ 137,  -8, 369 });
		AddPointToLine({ 119,  -1, 386 });
		AddPointToLine({ 135,   9, 399 });
		AddPointToLine({ 163,   7, 414 });
		AddPointToLine({ 172,   1, 426 });

		// 3E
		NewLine();
		AddPointToLine({ 181,  -2, 416 });
		AddPointToLine({ 169,  -7, 431 });
		AddPointToLine({ 153,  -8, 444 });
		AddPointToLine({ 132,  -1, 461 });
		AddPointToLine({ 119,   6, 474 });
		AddPointToLine({ 143,   9, 491 });
		AddPointToLine({ 173,   4, 504 });
		AddPointToLine({ 182,  -3, 519 });
		AddPointToLine({ 176,  -5, 531 });

		// 3F
		NewLine();
		AddPointToLine({ 159,  -8, 521 });
		AddPointToLine({ 145,  -4, 529 });
		AddPointToLine({ 126,   2, 546 });
		AddPointToLine({ 117,   8, 559 });
		AddPointToLine({ 138,   5, 576 });
		AddPointToLine({ 161,  -2, 587 });
		AddPointToLine({ 150,  -5, 600 });
		
		// ============================================================
// ЛИНИЯ 2 — ЦЕНТРАЛЬНАЯ СПИРАЛЬНАЯ ВОРОНКА
// ВСТАВИТЬ ПОСЛЕ 2C И ПЕРЕД 2D
// ============================================================


// ------------------------------------------------------------
// ВХОД В ВОРОНКУ ИЗ ЛИНИИ 2
// От ближайшей внутренней точки 2C
// ------------------------------------------------------------

		NewLine();

		AddPointToLine({ 127,   8, 251 });
		AddPointToLine({ 120,   7, 258 });
		AddPointToLine({ 118,   6, 266 });
		AddPointToLine({ 120,   5, 274 });
		AddPointToLine({ 128,   5, 280 });
		AddPointToLine({ 138,   5, 284 });


		// ------------------------------------------------------------
		// ВТОРОЙ ВХОД
		// ------------------------------------------------------------

		NewLine();

		AddPointToLine({ 156,   6, 264 });
		AddPointToLine({ 150,   7, 270 });
		AddPointToLine({ 145,   6, 276 });
		AddPointToLine({ 142,   5, 280 });
		AddPointToLine({ 138,   5, 284 });


		// ------------------------------------------------------------
		// ТРЕТИЙ ВХОД
		// ------------------------------------------------------------

		NewLine();

		AddPointToLine({ 179,  -1, 281 });
		AddPointToLine({ 170,   0, 283 });
		AddPointToLine({ 160,   2, 284 });
		AddPointToLine({ 150,   4, 284 });
		AddPointToLine({ 138,   5, 284 });



		// ============================================================
		// СПИРАЛЬНАЯ ВОРОНКА — НИТЬ 1
		// ============================================================

		NewLine();

		AddPointToLine({ 138,   5, 284 });

		AddPointToLine({ 155,   3, 288 });
		AddPointToLine({ 170,   0, 298 });
		AddPointToLine({ 181,  -5, 313 });
		AddPointToLine({ 184, -11, 330 });

		AddPointToLine({ 179, -17, 347 });
		AddPointToLine({ 166, -23, 360 });
		AddPointToLine({ 150, -29, 366 });
		AddPointToLine({ 134, -35, 362 });

		AddPointToLine({ 121, -41, 350 });
		AddPointToLine({ 115, -47, 334 });
		AddPointToLine({ 117, -53, 317 });
		AddPointToLine({ 127, -59, 304 });

		AddPointToLine({ 141, -65, 297 });
		AddPointToLine({ 156, -71, 299 });
		AddPointToLine({ 167, -77, 310 });
		AddPointToLine({ 172, -83, 325 });

		AddPointToLine({ 168, -89, 340 });
		AddPointToLine({ 157, -95, 350 });
		AddPointToLine({ 143,-101, 352 });
		AddPointToLine({ 132,-107, 344 });

		AddPointToLine({ 126,-113, 331 });
		AddPointToLine({ 128,-119, 318 });
		AddPointToLine({ 137,-125, 310 });
		AddPointToLine({ 148,-131, 311 });

		AddPointToLine({ 157,-137, 319 });
		AddPointToLine({ 160,-143, 330 });
		AddPointToLine({ 155,-149, 339 });
		AddPointToLine({ 146,-155, 342 });

		AddPointToLine({ 139,-161, 337 });
		AddPointToLine({ 137,-167, 328 });
		AddPointToLine({ 143,-173, 321 });

		AddPointToLine({ 150,-179, 320 });




		// ============================================================
		// СПИРАЛЬНАЯ ВОРОНКА — НИТЬ 2
		// ПРОТИВОПОЛОЖНОЕ НАПРАВЛЕНИЕ
		// ============================================================

		NewLine();

		AddPointToLine({ 138,   5, 284 });

		AddPointToLine({ 128,   4, 298 });
		AddPointToLine({ 123,   1, 315 });
		AddPointToLine({ 126,  -4, 333 });
		AddPointToLine({ 137, -10, 347 });

		AddPointToLine({ 152, -16, 356 });
		AddPointToLine({ 169, -22, 354 });
		AddPointToLine({ 181, -28, 342 });
		AddPointToLine({ 184, -34, 325 });

		AddPointToLine({ 179, -40, 309 });
		AddPointToLine({ 166, -46, 298 });
		AddPointToLine({ 151, -52, 296 });
		AddPointToLine({ 137, -58, 304 });

		AddPointToLine({ 130, -64, 317 });
		AddPointToLine({ 132, -70, 331 });
		AddPointToLine({ 142, -76, 340 });
		AddPointToLine({ 155, -82, 341 });

		AddPointToLine({ 165, -88, 333 });
		AddPointToLine({ 169, -94, 321 });
		AddPointToLine({ 164,-100, 311 });
		AddPointToLine({ 154,-106, 307 });

		AddPointToLine({ 143,-112, 312 });
		AddPointToLine({ 137,-118, 322 });
		AddPointToLine({ 140,-124, 332 });
		AddPointToLine({ 149,-130, 337 });

		AddPointToLine({ 157,-136, 332 });
		AddPointToLine({ 160,-142, 323 });
		AddPointToLine({ 155,-148, 316 });
		AddPointToLine({ 148,-154, 315 });

		AddPointToLine({ 142,-160, 320 });
		AddPointToLine({ 141,-166, 327 });
		AddPointToLine({ 146,-172, 331 });

		AddPointToLine({ 150,-179, 328 });




		// ============================================================
		// СПИРАЛЬНАЯ ВОРОНКА — НИТЬ 3
		// ТРЕТЬЯ ТРАЕКТОРИЯ ДЛЯ ЭФФЕКТА УРАГАНА
		// ============================================================

		NewLine();

		AddPointToLine({ 138,   5, 284 });

		AddPointToLine({ 132,   3, 300 });
		AddPointToLine({ 136,   0, 317 });
		AddPointToLine({ 147,  -5, 331 });
		AddPointToLine({ 162, -11, 337 });

		AddPointToLine({ 175, -17, 332 });
		AddPointToLine({ 181, -23, 319 });
		AddPointToLine({ 178, -29, 304 });
		AddPointToLine({ 166, -35, 294 });

		AddPointToLine({ 151, -41, 292 });
		AddPointToLine({ 137, -47, 299 });
		AddPointToLine({ 129, -53, 312 });
		AddPointToLine({ 130, -59, 326 });

		AddPointToLine({ 140, -65, 335 });
		AddPointToLine({ 153, -71, 336 });
		AddPointToLine({ 164, -77, 328 });
		AddPointToLine({ 168, -83, 317 });

		AddPointToLine({ 163, -89, 307 });
		AddPointToLine({ 153, -95, 302 });
		AddPointToLine({ 143,-101, 306 });
		AddPointToLine({ 137,-107, 316 });

		AddPointToLine({ 139,-113, 326 });
		AddPointToLine({ 148,-119, 332 });
		AddPointToLine({ 157,-125, 329 });
		AddPointToLine({ 161,-131, 320 });

		AddPointToLine({ 157,-137, 313 });
		AddPointToLine({ 149,-143, 311 });
		AddPointToLine({ 143,-149, 316 });
		AddPointToLine({ 142,-155, 323 });

		AddPointToLine({ 147,-161, 328 });
		AddPointToLine({ 153,-167, 327 });
		AddPointToLine({ 156,-173, 322 });

		AddPointToLine({ 150,-179, 320 });

//
//NewLine();
//
//AddPointToLine({ 150,-179, 320 });
//AddPointToLine({ 151,-188, 319 });
//AddPointToLine({ 152,-198, 318 });
//AddPointToLine({ 152,-208, 317 });
//AddPointToLine({ 151,-218, 316 });
//AddPointToLine({ 150,-228, 315 });
//
//
//// Нить 2
//
//NewLine();
//
//AddPointToLine({ 150,-179, 328 });
//AddPointToLine({ 151,-188, 326 });
//AddPointToLine({ 152,-198, 323 });
//AddPointToLine({ 152,-208, 320 });
//AddPointToLine({ 151,-218, 317 });
//AddPointToLine({ 150,-228, 315 });
//
//
//// Нить 3
//
//NewLine();
//
//AddPointToLine({ 150,-179, 320 });
//AddPointToLine({ 149,-188, 320 });
//AddPointToLine({ 149,-198, 319 });
//AddPointToLine({ 149,-208, 318 });
//AddPointToLine({ 150,-218, 316 });
//AddPointToLine({ 150,-228, 315 });
		


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
