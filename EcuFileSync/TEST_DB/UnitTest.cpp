#include "stdafx.h"

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

namespace TEST_DB
{
	[TestClass]
	public ref class UnitTest
	{
	private:
		TestContext^ testContextInstance;

	public: 
		/// <summary>
		///���� �׽�Ʈ ���࿡ ���� ���� �� �����
		///�����ϴ� �׽�Ʈ ���ؽ�Ʈ�� �������ų� �����մϴ�.
		///</summary>
		property Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ TestContext
		{
			Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ get()
			{
				return testContextInstance;
			}
			System::Void set(Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ value)
			{
				testContextInstance = value;
			}
		};

		#pragma region Additional test attributes
		//
		//�׽�Ʈ�� �ۼ��� �� ���� �߰� Ư���� ����� �� �ֽ��ϴ�.
		//
		//ClassInitialize�� ����Ͽ� Ŭ������ ù ��° �׽�Ʈ�� �����ϱ� ���� �ڵ带 �����մϴ�.
		//[ClassInitialize()]
		//static void MyClassInitialize(TestContext^ testContext) {};
		//
		//ClassCleanup�� ����Ͽ� Ŭ������ �׽�Ʈ�� ��� ������ �Ŀ� �ڵ带 �����մϴ�.
		//[ClassCleanup()]
		//static void MyClassCleanup() {};
		//
		//TestInitialize�� ����Ͽ� �� �׽�Ʈ�� �����ϱ� ���� �ڵ带 �����մϴ�.
		//[TestInitialize()]
		//void MyTestInitialize() {};
		//
		//TestCleanup�� ����Ͽ� �� �׽�Ʈ�� �����ϱ� ���� �ڵ带 �����մϴ�.
		//[TestCleanup()]
		//void MyTestCleanup() {};
		//
		#pragma endregion 

		[TestMethod]
		void TestMethod1()
		{
			//
			// TODO: �׽�Ʈ ���� ���⿡ �߰��մϴ�.
			//
		};
	};
}
