#include "stdafx.h"

using namespace System::Reflection;
using namespace System::Runtime::CompilerServices;
using namespace System::Runtime::InteropServices;

//
// ������� �Ϲ� ������ ���� Ư�� ������ ���� ����˴ϴ�.
// ������� ���õ� ������ �����Ϸ���
// �� Ư�� ���� �����Ͻʽÿ�.
//
[assembly:AssemblyTitleAttribute("TEST_DB")];
[assembly:AssemblyDescriptionAttribute("")];
[assembly:AssemblyConfigurationAttribute("")];
[assembly:AssemblyCompanyAttribute("Microsoft")];
[assembly:AssemblyProductAttribute("TEST_DB")];
[assembly:AssemblyCopyrightAttribute("Copyright (c) Microsoft 2017")];
[assembly:AssemblyTrademarkAttribute("")];
[assembly:AssemblyCultureAttribute("")];

//
// ������� ���� ������ ���� �� ���� ������ �����˴ϴ�.
//
//      �� ����
//      �� ����
//      ���� ��ȣ
//      ���� ����
//
// ��� ���� �����ϰų� �Ʒ��� ���� '*'�� ����Ͽ� ���� ��ȣ �� ���� ������ �ڵ�����
// �����ǵ��� �� �� �ֽ��ϴ�.

[assembly:AssemblyVersionAttribute("1.0.*")];
[assembly:ComVisible(false)];

