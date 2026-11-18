Imports System
Imports System.Runtime.InteropServices

Module Example06
    <DllImport("basicgol", EntryPoint:="bg_mensagem_utf8", CallingConvention:=CallingConvention.Cdecl)>
    Private Function MensagemUtf8() As IntPtr
    End Function

    Public Sub Run()
        Dim texto As String = Marshal.PtrToStringUTF8(MensagemUtf8())
        Console.WriteLine(texto)
    End Sub
End Module
