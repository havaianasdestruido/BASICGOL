Imports System
Imports System.Runtime.InteropServices

Module Example01
    <DllImport("basicgol", EntryPoint:="bg_soma", CallingConvention:=CallingConvention.Cdecl)>
    Private Function Somar(a As Integer, b As Integer) As Integer
    End Function

    Public Sub Run()
        Console.WriteLine($"BASICGOL: 19 + 23 = {Somar(19, 23)}")
    End Sub
End Module
