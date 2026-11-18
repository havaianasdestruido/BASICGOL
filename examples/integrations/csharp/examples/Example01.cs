using System;
using System.Runtime.InteropServices;

namespace Basicgol.Interop.CSharp;

internal static class Example01
{
    [DllImport("basicgol", EntryPoint = "bg_soma", CallingConvention = CallingConvention.Cdecl)]
    private static extern int Somar(int a, int b);

    public static void Run() => Console.WriteLine($"BASICGOL: 19 + 23 = {Somar(19, 23)}");
}
