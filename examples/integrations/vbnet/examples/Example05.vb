Imports System
Imports System.Runtime.InteropServices

Module Example05
    <DllImport("basicgol", EntryPoint:="bg_comprimento_utf8", CallingConvention:=CallingConvention.Cdecl)>
    Private Function ComprimentoUtf8(
        <MarshalAs(UnmanagedType.LPUTF8Str)> texto As String) As Integer
    End Function

    Public Sub Run()
        Dim texto As String = "ação"
        Console.WriteLine($"'{texto}' tem {ComprimentoUtf8(texto)} bytes UTF-8.")
    End Sub
End Module
