using System;
using System.Runtime.InteropServices;

namespace Basicgol.Interop.CSharp;

internal static class Example03
{
    [DllImport("basicgol", EntryPoint = "bg_media3", CallingConvention = CallingConvention.Cdecl)]
    private static extern double Media3(double a, double b, double c);

    public static void Run() => Console.WriteLine($"Media BASICGOL = {Media3(7.5, 8.5, 9.0):F2}");
}
