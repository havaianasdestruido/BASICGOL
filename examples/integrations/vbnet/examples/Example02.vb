Imports System
Imports System.Runtime.InteropServices

Module Example02
    <DllImport("basicgol", EntryPoint:="bg_fatorial", CallingConvention:=CallingConvention.Cdecl)>
    Private Function Fatorial(n As Integer) As Long
    End Function

    Public Sub Run()
        Console.WriteLine($"10! = {Fatorial(10)}")
    End Sub
End Module
