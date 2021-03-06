#region Copyright notice and license

// Copyright 2015, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#endregion

using System;
using System.Runtime.CompilerServices;
using System.Threading.Tasks;

namespace Grpc.Core
{
    /// <summary>
    /// Return type for bidirectional streaming calls.
    /// </summary>
    public sealed class AsyncDuplexStreamingCall<TRequest, TResponse>
        where TRequest : class
        where TResponse : class
    {
        readonly IClientStreamWriter<TRequest> requestStream;
        readonly IAsyncStreamReader<TResponse> responseStream;

        public AsyncDuplexStreamingCall(IClientStreamWriter<TRequest> requestStream, IAsyncStreamReader<TResponse> responseStream)
        {
            this.requestStream = requestStream;
            this.responseStream = responseStream;
        }

        /// <summary>
        /// Writes a request to RequestStream.
        /// </summary>
        public Task Write(TRequest message)
        {
            return requestStream.Write(message);
        }

        /// <summary>
        /// Closes the RequestStream.
        /// </summary>
        public Task Close()
        {
            return requestStream.Close();
        }

        /// <summary>
        /// Reads a response from ResponseStream.
        /// </summary>
        /// <returns></returns>
        public Task<TResponse> ReadNext()
        {
            return responseStream.ReadNext();
        }

        /// <summary>
        /// Async stream to read streaming responses.
        /// </summary>
        public IAsyncStreamReader<TResponse> ResponseStream
        {
            get
            {
                return responseStream;
            }
        }

        /// <summary>
        /// Async stream to send streaming requests.
        /// </summary>
        public IClientStreamWriter<TRequest> RequestStream
        {
            get
            {
                return requestStream;
            }
        }
    }
}
