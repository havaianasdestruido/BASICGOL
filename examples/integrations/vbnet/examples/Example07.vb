Imports System
Imports System.Runtime.InteropServices

Module Example07
    <UnmanagedFunctionPointer(CallingConvention.Cdecl)>
    Private Delegate Function Transformar(valor As Integer) As Integer

    <DllImport("basicgol", EntryPoint:="bg_aplica_callback", CallingConvention:=CallingConvention.Cdecl)>
    Private Function AplicarCallback(valor As Integer, callback As Transformar) As Integer
    End Function

    Private Function Triplicar(valor As Integer) As Integer
        Return valor * 3
    End Function

    Public Sub Run()
        Dim callback As Transformar = AddressOf Triplicar
        Console.WriteLine($"Callback VB.NET chamado por BASICGOL: {AplicarCallback(14, callback)}")
        GC.KeepAlive(callback)
    End Sub
End Module
