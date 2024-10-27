#pragma once

BOOL ShiftJIS_IsLeadByte( const char chByte );
BOOL ShiftJIS_IsTrailByte( const char chByte );
int ShiftJIS_StringCompareCI( LPCSTR szStringLeft, LPCSTR szStringRight, size_t sizeLength );
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
