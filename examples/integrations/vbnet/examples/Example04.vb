Imports System
Imports System.Runtime.InteropServices

Module Example04
    <DllImport("basicgol", EntryPoint:="bg_soma_vetor", CallingConvention:=CallingConvention.Cdecl)>
    Private Function SomarVetor(
        <MarshalAs(UnmanagedType.LPArray, SizeParamIndex:=1)> valores As Integer(),
        quantidade As Integer) As Integer
    End Function

    Public Sub Run()
        Dim valores As Integer() = {4, 8, 15, 16, 23, 42}
        Console.WriteLine($"Soma do array gerenciado = {SomarVetor(valores, valores.Length)}")
    End Sub
End Module
