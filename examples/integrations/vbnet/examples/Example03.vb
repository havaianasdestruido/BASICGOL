Imports System
Imports System.Runtime.InteropServices

Module Example03
    <DllImport("basicgol", EntryPoint:="bg_media3", CallingConvention:=CallingConvention.Cdecl)>
    Private Function Media3(a As Double, b As Double, c As Double) As Double
    End Function

    Public Sub Run()
        Console.WriteLine($"Media BASICGOL = {Media3(7.5, 8.5, 9.0):F2}")
    End Sub
End Module
