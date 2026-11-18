using System;
using System.Runtime.InteropServices;

namespace Basicgol.Interop.CSharp;

internal static class Example04
{
    [DllImport("basicgol", EntryPoint = "bg_soma_vetor", CallingConvention = CallingConvention.Cdecl)]
    private static extern int SomarVetor(
        [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] int[] valores, int quantidade);

    public static void Run()
    {
        int[] valores = { 4, 8, 15, 16, 23, 42 };
        Console.WriteLine($"Soma do array gerenciado = {SomarVetor(valores, valores.Length)}");
    }
}
