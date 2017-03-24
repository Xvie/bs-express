type done_;
/** abstract type which ensure middleware function must either
    call the [next] function or one of the [send] function on the 
    response object. 

    This should be a great argument for OCaml, the type system 
    prevents silly error which in this case would make the server hang */ 

/** TODO : maybe this should be a more common module like bs-error */
module Error = {

  type t;
  /** Error type */

  external message : t => option string = "" 
    [@@bs.send][@@bs.return null_undefined_to_opt];
  external name : t => option string = "" 
    [@@bs.send][@@bs.return null_undefined_to_opt];
};

module Request = {
  type t;
  
  type params = Js_dict.t Js_json.t;

  external params : t => params = "" [@@bs.get];
  /** [params request] return the JSON object filled with the 
      request parameters */

  external asJSONObject : t => Js_dict.t Js_json.t = "%identity";
  /** [asJSONObject request] casts a [request] to a JSON object. It is 
      common in Express application to use the Request object as a 
      placeholder to maintain state through the various middleware which 
      are executed. */ 

  /** more property method coming later */
};

module Response = {
  type t;
  external sendFile : t => string => 'a => done_ = "" [@@bs.send];
  external sendString : t => string => done_ = "send" [@@bs.send];
  external sendJson : t => Js.Json.t => done_ = "send" [@@bs.send];
  external sendBuffer : t => Buffer.t => done_ = "send" [@@bs.send];
  external sendArray : t => array 'a => done_ = "send" [@@bs.send];
  external json : t => Js.Json.t => done_ = "" 
    [@@bs.send] [@@ocaml.deprecated "Use sendJson instead`"];
};

module Next : {
  type content;

  type t = (Js.undefined content => done_) [@bs];

  let router : Js.undefined content;  
  /** value to use as [next] callback argument to skip middleware 
      processing. */ 

  let error : Error.t => Js.undefined content;
  /** [error e] returns the argument for [next] callback to be propagate 
      error [e] through the chain of middleware. */

} = {

  type content;

  type t = (Js.undefined content => done_) [@bs];

  external castToContent : 'a => content = "%identity";

  let router = 
    Js.Undefined.return (castToContent "router");

  let error (e:Error.t) =>
    Js.Undefined.return (castToContent e);

};

module Middleware = {
  type next = Next.t;
  type t; 
      /* Middleware abstract type which unified the various way an
         Express middleware can be constructed:
         {ul
         {- from a {b function} using the [ofFunction]}
         {- from a Router class}
         {- from an App class}
         {- from the third party middleware modules}
         }
         For each of the class which implements the middleware interface
         is JavaScript, one can use the "%identity" function to cast
         it to this type [t] */
  type f = Request.t => Response.t => next => done_;
  
  external fromF: f => t = "%identity"
    [@@ocaml.doc " Create a Middleware from a function " ]; 
  
  type errorF = Error.t => Request.t => Response.t => next => done_;
  
  external fromErrorF: errorF => t = "%identity"
    [@@ocaml.doc " Create a Middleware from an error function " ]; 
};

/* Generate the common Middleware binding function for a given
 * type. This Functor is used for the Router and App classes. */
module MakeBindFunctions (T: {type t;}) => {
  external use : T.t => Middleware.t => unit = "" [@@bs.send];
  external useOnPath : T.t => path::string => Middleware.t => unit = "use" [@@bs.send];
  external get : T.t => path::string => Middleware.t => unit = "" [@@bs.send];
  external getN : T.t => path::string => array Middleware.t => unit = "get" [@@bs.send];
};

module App = {
  type t;

  include MakeBindFunctions { type nonrec t = t; };
  
  external make : unit => t = "express" [@@bs.module]
    [@@ocaml.doc " [make ()] create an App class "]; 
  

  external toMiddleware : t => Middleware.t = "%identity"
    [@@ocaml.doc " [toMiddleware app] casts an App  to a Middleware type "]; 
  
  external listen : t => port::int => unit = "" [@@bs.send];
};
  
let express = App.make
  [@@ocaml.doc " [express ()] create an App class "]; 

module Static = {
  type options;

  let defaultOptions : unit => options =
      fun () => (Obj.magic (Js_obj.empty ()): options);
  
  external dotfiles : options => string => unit = "" [@@bs.set];
  external etag : options => Js.boolean => unit = "" [@@bs.set]; 
  
  /* ... add all the other options */

  type t;
  
  external make : string => options => t = "static" [@@bs.module "express"]
    [@@ocaml.doc " [make directory] create a static middleware for [directory] " ]; 

  external toMiddleware : t => Middleware.t = "%identity" 
    [@@ocaml.doc " Cast a Static class to a Middleware " ]; 
};
