﻿#include "stdafx.h"
#include "ModelAdvancedDlg.h"
using Whisper::eGpuModelFlags;

static void __stdcall addGpu( const wchar_t* name, void* pv )
{
	CComboBox& cb = *(CComboBox*)pv;
	cb.AddString( name );
}

LRESULT ModelAdvancedDlg::onInitDialog( UINT nMessage, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	cbWave = GetDlgItem( IDC_WAVE );
	cbReshapedMatMul = GetDlgItem( IDC_RESHAPED_MAT_MUL );
	cbAdapter = GetDlgItem( IDC_GPU );
	const uint32_t flags = appState.gpuFlagsLoad();

	// Setup the "Compute shaders" combobox
	cbWave.AddString( L"使用AMD处理器渲染 Wave64" );
	cbWave.AddString( L"总是使用 Wave32" );
	cbWave.AddString( L"总是使用 Wave64" );
	int i = 0;
	if( 0 != ( flags & (uint32_t)eGpuModelFlags::Wave32 ) )
		i = 1;
	else if( 0 != ( flags & (uint32_t)eGpuModelFlags::Wave64 ) )
		i = 2;
	cbWave.SetCurSel( i );

	// Setup the "Reshaped multiply" combobox
	cbReshapedMatMul.AddString( L"基于AMD处理" );
	cbReshapedMatMul.AddString( L"不使用张量处理" );
	cbReshapedMatMul.AddString( L"使用张量处理" );
	i = 0;
	if( 0 != ( flags & (uint32_t)eGpuModelFlags::NoReshapedMatMul ) )
		i = 1;
	else if( 0 != ( flags & (uint32_t)eGpuModelFlags::UseReshapedMatMul ) )
		i = 2;
	cbReshapedMatMul.SetCurSel( i );

	cbAdapter.AddString( L"使用默认值" );
	HRESULT hr = Whisper::listGPUs( &addGpu, &cbAdapter );
	if( FAILED( hr ) )
	{
		reportError( m_hWnd, L"无法获取到GPUs", L"", hr );
		return 0;
	}
	const CString setting = appState.stringLoad( L"gpu" );
	if( setting.GetLength() <= 0 )
		cbAdapter.SetCurSel( 0 );
	else
		cbAdapter.SetCurSel( cbAdapter.FindStringExact( 1, setting ) );

	return 0;
}

bool ModelAdvancedDlg::show( HWND owner )
{
	auto res = DoModal( owner );
	return res == IDOK;
}

void ModelAdvancedDlg::onOk()
{
	// Gather values from these comboboxes
	uint32_t flags = 0;

	int i = cbWave.GetCurSel();
	if( 1 == i )
		flags |= (uint32_t)eGpuModelFlags::Wave32;
	else if( 2 == i )
		flags |= (uint32_t)eGpuModelFlags::Wave64;

	i = cbReshapedMatMul.GetCurSel();
	if( 1 == i )
		flags |= (uint32_t)eGpuModelFlags::NoReshapedMatMul;
	else if( 2 == i )
		flags |= (uint32_t)eGpuModelFlags::UseReshapedMatMul;

	// Save to registry
	appState.gpuFlagsStore( flags );

	int gpuIndex = cbAdapter.GetCurSel();
	if( gpuIndex <= 0 )
		appState.stringStore( L"gpu", nullptr );
	else
	{
		CString gpu;
		cbAdapter.GetLBText( gpuIndex, gpu );
		appState.stringStore( L"gpu", gpu );
	}

	EndDialog( IDOK );
}