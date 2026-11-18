Imports System

Module Program
    Public Function Main(args As String()) As Integer
        Dim id As String = If(args.Length = 0, "--list", args(0))
        Select Case id
            Case "--list"
                Console.WriteLine("Escolha um exemplo: 01, 02, 03, 04, 05, 06 ou 07.")
            Case "01" : Example01.Run()
            Case "02" : Example02.Run()
            Case "03" : Example03.Run()
            Case "04" : Example04.Run()
            Case "05" : Example05.Run()
            Case "06" : Example06.Run()
            Case "07" : Example07.Run()
            Case Else
                Console.Error.WriteLine($"Exemplo desconhecido: {id}")
                Return 2
        End Select
        Return 0
    End Function
End Module
